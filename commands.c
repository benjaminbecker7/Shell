#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "background.h"
#include "commands.h"
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
    printf("\texit:\tTo exit the shell, enter the command \"exit\"\n");
    printf("\tcd:\tTo change the current directory, enter \"cd\" followed\n\
        \tby the directory or path you would like to be in\n");
    printf("\tgetcwd:\tTo view your current working directory, enter \"getcwd\"\n");
    printf("\tjobs:\tTo view the processes currently running in the background,\n\
        \tenter the command \"jobs\"\n");
    printf("\tkill:\tTo kill a background process currently running, enter the\n\
        \tcommand \"kill\" followed by the number of the process\n\
        \tyou would like to kill\n\n");

    printf("\033[1mExternal Commands:\033[0m\n");
    printf("\tTo run a command off of a file, enter the file path\n");
    printf("\tTo run a command off of a file in the background, enter\n\
        \tthe file path followed by an ampersand (&)\n\n");

    printf("To view this screen again, enter \"help\"\n");
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

void cmd_getcwd() {
    printf("You are currently in %s\n", strrchr(getcwd(NULL, MAX_PATH_LENGTH), '/'));
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
        printf("\033[0;31mBackground Process Error: Process %d not found\033[0m\n", bpid);
    }
}

//****************************************************************************

//*****************EXTERNAL COMMANDS******************************************

void cmd_extern(char ** command) {
    int pid = fork();
    if(pid == 0) { // child process
        if(execv(command[0], command) < 0) {
            printf("\033[0;31mExternal Command Error: File \"%s\" not found\033[0m\n", command[0]);
        }
        exit(0);
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
            printf("\033[0;31mExternal Command Error: File \"%s\" not found\033[0m\n", command[0]);
        }
        exit(0);
    } else if(bpid > 0) { // parent process
        if(add_bp(bg, bpid, 0)) {
            printf("Created background process %d\n", bpid);
        } else {
            printf("\033[0;31mBackground Process Error: Not enough space in background, killing process\033[0m\n");
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
    } else if (!strcmp(command[0], "getcwd")) {
        cmd_getcwd();
    } else if (!strcmp(command[0], "cd")) {
        cmd_cd(command);
    } else if (!strcmp(command[0], "jobs")) { // command is a jobs cmd
        cmd_jobs(bg);
    } else if (!strcmp(command[0], "kill")) { // command is a kill cmd
        cmd_kill(command, bg);
    } else if (command[0][0] == '/') { // command is external
        int i = cmd_is_bg(command);
        if(i) { // command is background
            command[i] = NULL;
            cmd_extern_bg(command, bg);
        } else { // command is normal
            cmd_extern(command);
        }
    } else {
        printf("Command not found. Enter \"help\" for a list of available commands\n");
    }
}