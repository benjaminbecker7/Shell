#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "background.h"
#include "shell.h"

//*********************BUILT-IN COMMANDS*****************************************

/**
 * Frees occupied memory and exits the program.
 * @param command the command array that needs to be freed before exit
 * @param bg the list of background processes running at the end of the program
 */
void cmd_exit(char ** command, struct bpid_list * bg) {
    printf("Logging out...\n");
    free_tokens(command);
    free_background(bg);
    exit(0);
}

void cmd_help() {
    printf("\033[1m");
    printf("This is a shell by Benjamin Becker and Alex Wu\n\n");
    
    printf("Built-In Commands:\n");
    printf("\033[0m");
    printf("To exit the shell, enter the command \"exit\"\n");
    printf("To view the processes currently running in the background,\n\
        enter the command \"jobs\"\n");
    printf("To kill a background process currently running, enter the\n\
        command \"kill\" followed by the number of the process you\n\twould like to kill\n\n");

    printf("\033[1m");
    printf("External Commands:\n");
    printf("\033[0m");
    printf("To run a command off of a file, enter the file path\n");
    printf("To run a command off of a file in the background, enter\n\
        the file path followed by an ampersand (&)\n");
}

void cmd_cd(char ** command) {
    if(command[1] != NULL) {
        if(chdir(command[1]) != 0) {
            printf("Directory %s not found\n", command[1]);
        }
    } else {
        printf("cd: please provide a directory\n");
    }
}

void cmd_jobs(struct bpid_list * bg) {
    if(bg->size > 0) {
        printf("Processes currently active: ");
        struct bpid_list_node * current = bg->head;
        while(current->next != NULL) {
            printf("%d, ", current->bpid);
            current = current->next;
        }
        printf("%d\n", current->bpid);
    } else if(bg->size == 0) {
        printf("No background processes currently active\n");
    }
}

void cmd_kill(char ** command, struct bpid_list * bg) {
    int bpid = atoi(command[1]);
    if(remove_bp(bg, bpid)) {
        printf("Process %d has been killed\n", bpid);
        kill(bpid, SIGKILL);
    } else {
        printf("Process %d not found\n", bpid);
    }
}

//****************************************************************************

//*****************EXTERNAL COMMANDS******************************************

void cmd_extern(char ** command) {
    int pid = fork();
    if(pid == 0) { // child process
        if(execv(command[0], command) < 0) {
            fprintf(stderr, "File \"%s\" not found\n", command[0]);
        }
    } else if(pid > 0) { // parent process
        waitpid(pid, 0, 0);
    }
}

int cmd_is_bg(char ** command) {
    int i = 0;
    while(command[i+1] != NULL) {
        i++;
    }

    if(command[i][0] == '&') {
        return i;
    }
    
    return 0;
}

void cmd_extern_bg(char ** command, struct bpid_list * bg) {
    int bpid = fork();
    if(bpid == 0) { // child process
        
        if(execv(command[0], command) < 0) {
            fprintf(stderr, "File \"%s\" not found\n", command[0]);
        }
        exit(0);
    } else if(bpid > 0) { // parent process
        if(add_bp(bg, bpid, 0)) {
            printf("Created background process %d\n", bpid);
        } else {
            printf("Not enough space in background, killing process\n");
            kill(bpid, SIGKILL);
        }
    }
}

//*************************************************************************

void runcmd(char ** command, struct bpid_list * bg) {
    if(command[0] == NULL) return; // primary check to see if valid command

    if(!strcmp(command[0], "exit")) { // command is an exit cmd
        cmd_exit(command, bg);
    } else if (!strcmp(command[0], "help")) {
        cmd_help();
    } else if (!strcmp(command[0], "cd")) {
        cmd_cd(command);
    } else if (!strcmp(command[0], "jobs")) { // command is a jobs cmd
        cmd_jobs(bg);
    } else if (!strcmp(command[0], "kill")) { // command is a kill cmd
        cmd_kill(command, bg);
    } else { // command is external
        int i = cmd_is_bg(command);
        if(i) { // command is background
            command[i] = NULL;
            cmd_extern_bg(command, bg);
        } else { // command is normal
            cmd_extern(command);
        }
    }
}