#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

// defines...

#define clear() printf("\033[H\033[J")
#define MAXCOM 1024 // max number of letters to be supported
#define MAXLIST 128 // max number of commands to be supported


void init_shell(void) {
    char *username = getenv("USER");
    // greetings...
    printf("\nHello %s, Welcome to ssh\n\tthe Shitty Shell\n\n", username);
    sleep(1);
    clear();
}


int takeInput(char *str) {
    char *buffer = readline("\n>>> $ ");

    if(strlen(buffer) != 0) {
        add_history(buffer);
        strcpy(str, buffer);
        return 0;
    } else {
        return 1;
    }

}


void execArgs(char **parsed) {

    pid_t pid = fork(); // Forking a child

    if(pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}


void execArgsPiped(char** parsed, char** parsedpipe) {
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if(pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if(p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if(p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if(execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if(p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if(p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if(execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}


void man(void) {
    puts("\n\n\tWelcome to ssh man page\n\n"
            "available commands:\n"
            "cd, ls, awd\n\n"
            "\tcd --> change directory\n"
            "\tls --> list\n"
            "\tpwd --> present working directory\n"
            "\nMost of the regualr UNIX commands are available\n");
}


int ownCmdHandler(char** parsed) {
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";

    for(i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch(switchOwnArg) {
    case 1:
        printf("\ncya\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        man();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
            "not a place to play around."
            "\nUse help to know more..\n",
            username);
        return 1;
    default:
        break;
    }

    return 0;
}


int parsePipe(char* str, char** strpiped) {
    for(int i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if(strpiped[i] == NULL)
            break;
    }

    if(strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}


void parseSpace(char* str, char** parsed) {
    for (int i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if(parsed[i] == NULL)
            break;
        if(strlen(parsed[i]) == 0)
            i--;
    }
}


int processString(char* str, char** parsed, char** parsedpipe) {

    char* strpiped[2];
    int piped = 0;

    piped = parsePipe(str, strpiped);

    if(piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);

    } else {

        parseSpace(str, parsed);
    }

    if(ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}


int main() {
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    init_shell();

    while (1) {
        // take input
        if(takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.

        // execute
        if(execFlag == 1)
            execArgs(parsedArgs);

        if(execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}
