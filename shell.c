#include "parser/ast.h"
#include "shell.h"
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define STDIN  0
#define STDOUT 1
#define PIPE_RD 0
#define PIPE_WR 1

pid_t pidsInUse[10]; // this may cause issues if more than 10 commands are needed at any given time but for the purposes of the assignment it'll do

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void ctrlcHandle(int signal) {
    //printf("caught signal %d.", signal);
    for(int i = 0; i < 10; i++) if(pidsInUse[i] != 0) kill(pidsInUse[i], signal); // kill all child processes
    if (prompt)
        prompt = "\nvush$ ";
}

void addpid(pid_t pidToAdd) {
    int amount = 0;
    for(int i = 0; i < 10; i++) {
        if(pidsInUse[i] != 0) {
            amount++;
        }
    }
    pidsInUse[amount] = pidToAdd; // adds a PID to the end of array pidsInUse
}

void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);
    signal(SIGINT, ctrlcHandle);
    if(strcmp(node->command.program, "exit") == 0) { // catch exit
        for(int i = 0; i < 10; i++) if(pidsInUse[i] != 0) kill(pidsInUse[i], 2);
        int exitCode = 0; // default exit code
        if(node-> command.argv[1] != NULL) {
            exitCode = atoi(node-> command.argv[1]);
        }
        exit(exitCode);
    }
    if(strcmp(node->command.program, "cd") == 0) {
        chdir(node-> command.argv[1]);
        if(strcmp(node->command.argv[1], "/") != 0) { // if changing to root dir, don't print anything
            printf("%s", node-> command.argv[1]);
        }
        return;
    }
    switch(node->type) {
        case NODE_COMMAND:
            commandNormal(node);
            break;
        case NODE_PIPE:
            doPipe(node);
            break;
        case NODE_SEQUENCE:
            doSeq(node);
            break;
        default:
            break;
    }
}

void commandNormal(node_t *node) {
    pid_t pid = fork();
    addpid(pid);
    if(pid == 0) {
        //printf("Hello from child with pid %i\n!", getpid());
        if(execvp(node -> command.program, node-> command.argv) < 0) {
            perror(strerror(errno));
        }
    }
    else {
        waitpid(pid, NULL, 0);
        if (prompt)
        prompt = "vush$ ";
    }
}

void doPipe(node_t *node) {
    pid_t pid1, pid2;
    int fd[2];

    pipe(fd); // essentially the code from the slides

    pid1 = fork();
    addpid(pid1);
    if(pid1 == 0) {
        //printf("Hello from child with pid %i\n!", getpid());
        close(fd[PIPE_RD]);
        close(STDOUT);
        dup(fd[PIPE_WR]);
        execvp(node->pipe.parts[0]->command.program, node->pipe.parts[0]->command.argv);
    }

    pid2 = fork();
    addpid(pid2);
    if(pid2 == 0) {
        //printf("Hello from child with pid %i\n!", getpid());
        close(fd[PIPE_WR]);
        close(STDIN);
        dup(fd[PIPE_RD]);
        execvp(node->pipe.parts[1]->command.program, node->pipe.parts[1]->command.argv);
    }

    close(fd[PIPE_RD]);
    close(fd[PIPE_WR]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void doSeq(node_t *node) {
    run_command(node->sequence.first); // simply runs the first command and then the next - since it calls run_command(), this accounts for nested sequences too
    run_command(node->sequence.second);
}