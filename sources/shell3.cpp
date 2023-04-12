#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <iterator>
#include <list>

#include <iostream>
#include <stack>
#include <memory>
#include "Variable.hpp"
#include <vector>

#define Close(FD) do {                                  \
    int Close_fd = (FD);                                \
    if (close(Close_fd) == -1) {                        \
      perror("close");                                  \
      fprintf(stderr, "%s:%d: close(" #FD ") %d\n",     \
              __FILE__, __LINE__, Close_fd);            \
    }                                                   \
  }while(0)

using namespace std;
using std::vector;
using std::string;
char *prompt = "hello: ";
int his_count=0;
int browse_mod = 0;


int exec(char *command, int fix_bit);

pid_t pId;
vector<char *> myList;
bool flag = false;
std::vector<auto_ptr<Variable>> vars;
int cond_array[2][3] = {{-1, -1, -1}, {-1, -1, -1}};
int cond_mod = 0;
int cond_status = -1;
int last_status = 0;

int get_last_command_index(char *argv[10], int argc) {
    int index = 0;
    int i = 0;
    for (i = 0; i < argc; i++) {
        if (!strcmp(*(argv + i), "|")) {
            index = i;
        }
    }
    return index;
}

static int child = 0; /* whether it is a child process relative to main() */

static void report_error_and_exit(const char *msg) {
    perror(msg);
    (child ? _exit : exit)(EXIT_FAILURE);
}

/** move oldfd to newfd */
static void redirect(int oldfd, int newfd) {
    if (oldfd != newfd) {
        if (dup2(oldfd, newfd) != -1)
            Close(oldfd); /* successfully redirected */
        else
            report_error_and_exit("dup2");
    }
}

static void exec_pipeline(vector<vector<char *>> cmds, size_t pos, int in_fd) {
    int i = 0;
    char **argv = (char **) malloc(cmds.at(pos).size() * sizeof(char *));
    for (i = 0; i < cmds.at(pos).size(); i++) {
        argv[i] = cmds.at(pos).at(i);
    }
    if (cmds.size() == pos + 1) { /* last command */
        redirect(in_fd, STDIN_FILENO); /* read from in_fd, write to STDOUT */
        execvp(argv[0], argv);
        report_error_and_exit("execvp last");
    } else { /* $ <in_fd cmds[pos] >fd[1] | <fd[0] cmds[pos+1] ... */
        int fd[2]; /* output pipe */
        if (pipe(fd) == -1)
            report_error_and_exit("pipe");
        switch (fork()) {
            case -1:
                report_error_and_exit("fork");
            case 0: /* child: execute current command `cmds[pos]` */
                child = 1;
                Close(fd[0]); /* unused */
                redirect(in_fd, STDIN_FILENO);  /* read from in_fd */
                redirect(fd[1], STDOUT_FILENO); /* write to fd[1] */
                execvp(*argv, argv);
                report_error_and_exit("execvp");
            default: /* parent: execute the rest of the commands */
                Close(fd[1]); /* unused */
                Close(in_fd); /* unused */
                exec_pipeline(cmds, pos + 1, fd[0]); /* execute the rest */
        }
    }
}

void sigint_handler(int sig) {
    printf("You typed Control-C!\n");
    if(flag){
        kill(pId,SIGTERM);
    }
//    printf("%s",prompt);
    fflush(stdout);
}

int main() {
    signal(SIGINT, sigint_handler);
    char command[1024];
    vector<char *> myList;

    while (1) {
        printf("%s", prompt);
        fgets(command, 1024, stdin);

        int checkQuit = exec(command, 0);
        last_status = checkQuit;
        //if this is the output of the if condition
        if(cond_mod == 1 && cond_array[0][0] == 1 && cond_array[0][1] == -1 && cond_array[0][2] == -1){
            cond_status = checkQuit;
        }
        if (checkQuit == 7) return 0;
    }
}

int exec(char *command, int fix_bit) {
    int fd, amper, redirect, redirect2, redirect3, piping, retid, status, argc1, i;
    int pipe_counter = 1;
    int fildes[2];
    char *argv1[10], *argv2[10], *save[20];
    vector<vector<char *>> commands;
    char *outfile;
    char *token;

    if(fix_bit == 0) {
        command[strlen(command) - 1] = '\0';
    }
    piping = 0;

    char *tempStr = new char[strlen(command)];
    strcpy(tempStr, command);

    /* parse command line */
    i = 0;
    token = strtok(command, " ");
    commands.emplace_back();
    while (token != NULL) {
        argv1[i] = token;
        if(strcmp(token, "|") && strcmp(token, "if")) {//do not include if or | in the commands
            commands.at(pipe_counter - 1).push_back(token);
        }
        token = strtok(NULL, " ");
        i++;
        if (token && !strcmp(token, "|")) {
            commands.at(pipe_counter-1).emplace_back();
            commands.emplace_back();
            pipe_counter++;
            piping = 1;
        }
    }
//    commands.pop_back();
    argv1[i] = NULL;
    argc1 = i;

    /* Is command empty */
    if (argv1[0] == NULL) {
        if(browse_mod == 1){
            int status = 0;
            status = exec(myList.at(myList.size()-1-his_count), 1);
            his_count = 0;
            browse_mod = 0;
            return status;
        }
        return 0;
    }
    //for q 12
    if((!strcmp(argv1[0], "\033[B")) || (!strcmp(argv1[0], "\033[A"))){
        for(int j=0; j < argc1; j++){
            if(!strcmp(argv1[j],"\033[A")){
                if(his_count!=myList.size()-1){
                    if(browse_mod == 0){
                        browse_mod = 1;
                    }
                    else {
                        his_count++;
                    }
                }
            }
            else
            if(!strcmp(argv1[j],"\033[B") && browse_mod == 1){
                if(his_count!=0){
                    his_count--;
                }
            }

            else{ printf(" bad syntax\n");
                return 0;
            }

        }

        int ind = myList.size()-1-his_count;
        cout<<myList.at(ind)<<endl;
        cout.flush();
        return 0;
    }
//    his_count=0;

    //enter cond mod
    if(!strcmp(argv1[0], "if") && cond_mod == 0){
        cond_mod = 1;
        myList.push_back(tempStr);
        cond_array[0][0] = 1;
        cond_array[1][0] = myList.size()-1;
        return 0;
    }

    //enter the then input mod
    if((!strcmp(argv1[0], "then")) && cond_mod == 1){
        cond_array[0][1] = 0;
        myList.push_back(tempStr);
        return 0;
    }

    //get inputs in then input mod
    if(cond_array[0][1] == 0 && cond_mod == 1 && strcmp(argv1[0], "else")){
        myList.push_back(tempStr);
        cond_array[1][1] = myList.size()-1;
        return 0;
    }

    //enter else input mod
    if((!strcmp(argv1[0], "else")) && cond_mod == 1){
        myList.push_back(tempStr);
        if(cond_array[0][1] == 0){
            cond_array[0][1] = 1;
        }
        cond_array[0][2] = 0;
        return 0;
    }

    //get inputs in else input mod
    if(cond_mod == 1 && cond_array[0][2] == 0 && strcmp(argv1[0], "fi")){
        myList.push_back(tempStr);
        cond_array[1][2] = myList.size()-1;
        return 0;
    }

    //finish the condition and activate the code
    if(cond_mod == 1 && !strcmp(argv1[0], "fi")){
        myList.push_back(tempStr);
        cond_array[0][2] = 1;
        auto it = myList.begin();
        int k = 0;
        int ind;
        cond_mod = 0;
        ind = cond_array[1][0];// the index of the "if" command
        cond_status = exec(myList.at(ind), 1);
        if(cond_status == 0){
            ind = cond_array[1][0] + 2;//the index of the if + 1(then) + 1(the first command)
            while(k < ind){
                it = next(it);
                k++;
            }
            for(int k = ind; k <= cond_array[1][1]; k++){
                status = exec(*it, 1);
                it = next(it);
            }
        }
        else{
            ind = cond_array[1][1] + 2;//the index of the last command of 'then' + 1(else) +1(the first command)
            while(k < ind){
                it = next(it, 1);
                k++;
            }
            for(int k = ind; k <= cond_array[1][2]; k++){
                status = exec(*it, 1);
                it = next(it);
            }
        }
        cond_mod = 0;
        cond_array[0][0] = -1;
        cond_array[0][1] = -1;
        cond_array[0][2] = -1;
        cond_array[1][2] = -1;
        cond_array[1][1] = -1;
        cond_array[1][0] = -1;
        return status;
    }

    if(!strcmp(argv1[0], "if") && cond_mod == 1 && argc1 >= 2){
        int k = 1;
        for(k=1;k<=argc1;k++){
            argv1[k-1] = argv1[k];
        }
        argc1--;
    }


    //Variable assigning
    if (std::string(argv1[0]).rfind("$", 0) == 0) {
        if (!strcmp(argv1[1], "="))
            if (argc1 >= 3) {
                std::string key = std::string(argv1[0]);
                key = key.substr(1, key.length() - 1);
                std::string value = std::string(argv1[2]);
                Variable *v = new Variable(key, value);
                auto_ptr<Variable> var_ptr(v);
                vars.emplace_back(var_ptr);
            }
    }

    // for question 7
    if (!strcmp("quit", argv1[0])) { return 7; }

    //question 6
    if (!strcmp(argv1[0], "!!")) {
        if (myList.empty()) {
//            printf(" 444%s\n", myList.back());
            return 0;
        }
//        printf("5555 %s\n", myList.back());

        exec(myList.back(), 1);
        return 0;
    }
    // for question 6
    if (myList.empty() || strcmp(myList.back(), tempStr)) {
        myList.push_back(tempStr);
    }

    // for question 4 //
    if (argc1 == 2 && (!strcmp(argv1[0], "echo")) && (!strcmp(argv1[1], "$?"))) {
        printf("%d\n", last_status);
        return 0;
    }

    // for question 2: prompt
    if (argc1 == 3 && (!strcmp(argv1[0], "prompt")) && (!strcmp(argv1[1], "="))) {
        prompt = argv1[2];
        return 0;
    }

    /* Does command line end with & */
    if (!strcmp(argv1[argc1 - 1], "&")) {
        amper = 1;
        argv1[argc1 - 1] = NULL;
    } else
        amper = 0;

    // for question 3
    // edited in order to echo variables values
    if (argc1 > 1 && !strcmp(argv1[0], "echo")) {
        int flag = 0;
        if (argc1 == 2) {
            std::string key = argv1[1];
            if (key.rfind("$", 0) == 0) {
                key = key.substr(1, key.length() - 1);
                int k = 0;
                for (k = 0; k < vars.size(); k++) {
                    if (vars[k].get()->getKey().compare(key) == 0) {
                        cout << vars[k].get()->getValue() << endl;
                        flag = 1;
                    }
                }
            }
        }
        if (flag == 0) {
            int j = 1;
            while (j < i - 1) {
                printf("%s ", (argv1[j++]));
            }
            printf("%s\n", argv1[j]);
        }
        return 0;
    }
    // question 5
    if (argc1 == 2 && (!strcmp(argv1[0], "cd"))) {
        int check = chdir((command + 3));
        if (check == -1) {
            printf("chdir faild, not valid path");
        }
        return 0;
    }

    if (argc1 == 2 && (!strcmp(argv1[0], "read"))){
        int index = 0;
        for(index = 0; index<vars.size(); index++){
            if(vars.at(index).get()->getKey() == string(argv1[1])){
                cout<<vars.at(index).get()->getValue()<<endl;
            }
        }
    }

    redirect = 0;
    redirect2 = 0;
    redirect3 = 0;

    if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">")) {
        redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    } else
        // for question 1
    if (argc1 > 1 && !strcmp(argv1[argc1 - 2], "2>")) {
        redirect2 = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    } else if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">>")) {
        redirect3 = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    }
    if (piping == 1) {
        if (fork() == 0) {
            exec_pipeline(commands, 0, STDIN_FILENO);
        }
        wait(&status);
        return status;
    }

    /* for commands not part of the shell command language */
    flag = true;
    pId = fork();
    if (pId == 0) {
        /* redirection of IO ? */
        if (redirect) {
            fd = creat(outfile, 0660);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            /* stdout is now redirected */
        } else
            // for question 1
        if (redirect2) {
            fd = creat(outfile, 0660);
            close(STDERR_FILENO);
            dup(fd);
            close(fd);
            /* stderr is now redirected */
        } else if (redirect3) {
            FILE *file_pointer = fopen(outfile, "a");
            fd = fileno(file_pointer);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            /* append redirected */
        }
        execvp(argv1[0], argv1);
    }

/* parent continues over here... */
/* waits for child to exit if required */
    if (amper == 0)
        retid = wait(&status);

    flag = false;
    return 0;
}

