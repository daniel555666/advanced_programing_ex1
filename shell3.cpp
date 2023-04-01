#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

#include <iostream>
#include <stack>
#include <vector>
#include <memory>
#include "Variable.hpp"
using namespace std;

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}


int main()
{
    char command[1024];
    char *token;
    int i;
    char *outfile, *prompt = "hello: ";
    int fd, amper, redirect, redirect2, redirect3, piping, retid, status, argc1;
    int fildes[2];
    char *argv1[10], *argv2[10], *save[20];
    stack<char *> myStack;
    vector<auto_ptr<Variable *>> variables;



    while (1)
    {

        printf("%s", prompt);
        fgets(command, 1024, stdin);

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
            if (i == 1 && (!strcmp(argv1[0], "!!")))
            {
                // printf(tempStr);//maybe need
                i = 0;
                strcpy(command, myStack.top());
                free(tempStr);
                tempStr = new char[strlen(command)];
                strcpy(tempStr, myStack.top());
                token = strtok(command, " ");
            }
        }
        argv1[i] = NULL;
        argc1 = i;

        /* Is command empty */
        if (argv1[0] == NULL)
        {
            continue;
        }

        // for question 7
        if (myStack.empty() || strcmp(myStack.top(), tempStr))
        {
            myStack.push(tempStr);
        }

        // for question 4 //need to fix this
        if (argc1 == 2 && (!strcmp(argv1[0], "echo")) && (!strcmp(argv1[1], "$?")))
        {
            printf("%d\n", status);
            status = 0;
            continue;
        }

        // for question 2: prompt
        if (argc1 == 3 && (!strcmp(argv1[0], "prompt")) && (!strcmp(argv1[1], "=")))
        {
            prompt = argv1[2];
            continue;
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
            continue;
        }
        // question 5
        if (argc1 == 2 && (!strcmp(argv1[0], "cd")))
        {
            int check = chdir((command + 3));
            if (check == -1)
            {
                printf("chdir faild, not valid path");
            }
            continue;
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

        if (fork() == 0)
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
        }

        // question 10 - add a variable    
        if(i>=3 && prefix("$", argv1[0]) && !strcmp(argv1[1], "=")){
            std::string varName = std::string()
            std::string varVal = std::string()
            int index = 1;
            for(index = 1; i < strlen(argv1[0])){
                varName += argv1[0][index]; 
            }
            for(index = 1; i < strlen(argv1[0])){
                varVal += argv1[2][index];
            }
            Variable* v = new Variable(varName, varVal);
            auto_ptr<Variable *> ptr(v)
            variables.push_back(std::auto_ptr<Variable *>())
        }

}
