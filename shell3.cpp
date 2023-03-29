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
using namespace std;
char *prompt = "hello: ";
int exec(char *command);
pid_t pId;
list <char*>myList;
bool flag=false;

void sigint_handler(int sig) {
    
    printf("You typed Control-C!\n");
    if(flag){
        kill(pId,SIGTERM);
    }
    printf("%s",prompt);
    fflush(stdout);
}

int main()
{
    signal(SIGINT, sigint_handler);
    char command[1024];
    list<char *> myList;

    while (1)
    {
        printf("%s", prompt);
        fgets(command, 1024, stdin);
        int checkQuit=exec(command);
        if(checkQuit==7) return 0;
    }
}

int exec(char *command)
{
    int fd, amper, redirect, redirect2, redirect3, piping, retid, status, argc1, i;
    int fildes[2];
    char *argv1[10], *argv2[10], *save[20];
    char *outfile;
    char *token;

    command[strlen(command) - 1] = '\0';
    piping = 0;

    char *tempStr = new char[strlen(command)];
    strcpy(tempStr, command);

    /* parse command line */
    i = 0;
    token = strtok(command, " ");
    while (token != NULL)
    {
        argv1[i] = token;
        token = strtok(NULL, " ");
        i++;
        if (token && !strcmp(token, "|"))
        {
            piping = 1;
            break;
        }
    }
    argv1[i] = NULL;
    argc1 = i;

    /* Is command empty */
    if (argv1[0] == NULL)
    {
        return 0;
    }
    // for question 7
    if(!strcmp("quit",argv1[0])){return 7 ;}

    //question 6
    if(!strcmp(argv1[0], "!!")){
        if(myList.empty())
        {        printf(" 444%s\n",myList.back());
                    return 0;}
                printf("5555 %s\n",myList.back());

        exec(myList.back());
        return 0;
    }
     // for question 6
    if (myList.empty() || strcmp(myList.back(), tempStr))
    {
        printf("3333%s\n",tempStr);
        myList.push_back(tempStr);
    }

    // for question 4 //
    if (argc1 == 2 && (!strcmp(argv1[0], "echo")) && (!strcmp(argv1[1], "$?")))
    {
        int child_status = status & 0xFF;
        printf("%d\n", child_status);
        return 0;
    }

    // for question 2: prompt
    if (argc1 == 3 && (!strcmp(argv1[0], "prompt")) && (!strcmp(argv1[1], "=")))
    {
        prompt = argv1[2];
        return 0;
    }

    /* Does command contain pipe */
    if (piping)
    {
        i = 0;
        while (token != NULL)
        {
            token = strtok(NULL, " ");
            argv2[i] = token;
            i++;
        }
        argv2[i] = NULL;
    }

    /* Does command line end with & */
    if (!strcmp(argv1[argc1 - 1], "&"))
    {
        amper = 1;
        argv1[argc1 - 1] = NULL;
    }
    else
        amper = 0;

    // for question 3
    if (argc1 > 1 && !strcmp(argv1[0], "echo"))
    {
        int j = 1;
        while (j < i - 1)
        {
            printf("%s ", (argv1[j++]));
        }
        printf("%s\n", argv1[j]);
        return 0;
    }
    // question 5
    if (argc1 == 2 && (!strcmp(argv1[0], "cd")))
    {
        int check = chdir((command + 3));
        if (check == -1)
        {
            printf("chdir faild, not valid path");
        }
        return 0;
    }

    redirect = 0;
    redirect2 = 0;
    redirect3 = 0;

    if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">"))
    {
        redirect = 1;
        argv1[argc1 - 2] = NULL;
        outfile = argv1[argc1 - 1];
    }
    else
        // for question 1
        if (argc1 > 1 && !strcmp(argv1[argc1 - 2], "2>"))
        {
            redirect2 = 1;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }
        else if (argc1 > 1 && !strcmp(argv1[argc1 - 2], ">>"))
        {
            redirect3 = 1;
            argv1[argc1 - 2] = NULL;
            outfile = argv1[argc1 - 1];
        }

    /* for commands not part of the shell command language */
    flag=true;
    pId=fork();
    if (pId == 0)
    {
        /* redirection of IO ? */
        if (redirect)
        {
            fd = creat(outfile, 0660);
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
            /* stdout is now redirected */
        }
        else
            // for question 1
            if (redirect2)
            {
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
                dup(fd);
                close(fd);
                /* stderr is now redirected */
            }
            else if (redirect3)
            {
                FILE *file_pointer = fopen(outfile, "a");
                fd = fileno(file_pointer);
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* append redirected */
            }
        if (piping)
        {
            pipe(fildes);
            if (fork() == 0)
            {
                /* first component of command line */
                close(STDOUT_FILENO);
                dup(fildes[1]);
                close(fildes[1]);
                close(fildes[0]);
                /* stdout now goes to pipe */
                /* child process does command */
                execvp(argv1[0], argv1);
            }
            /* 2nd command component of command line */
            close(STDIN_FILENO);
            dup(fildes[0]);
            close(fildes[0]);
            close(fildes[1]);
            /* standard input now comes from pipe */
            execvp(argv2[0], argv2);
        }
        else
            execvp(argv1[0], argv1);
    }
    /* parent continues over here... */
    /* waits for child to exit if required */
    if (amper == 0)
        retid = wait(&status);

    flag=false;
    return 0;
}

