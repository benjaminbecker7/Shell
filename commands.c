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

/**
 * Command function that outputs to stream the explanations 
 * of available commands for the shell.
 */
void cmd_help() {
    printf("\033[1;36mThis is a shell by Benjamin Becker and Alex Wu\033[0m\n\n");

    printf("\033[1mBuilt-In Commands:\033[0m\n");
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

    printf("\033[1mRedirecting Output:\033[0m\n");
    printf("\tTo redirect the output of a command to a file, enter the\n\
        \tcommand followed by an arrow (\">\") followed by the\n\
        \tfilename\n\n");

    printf("To view this screen again, enter \"help\"\n");
}

/**
 * Command function that changes the current working directory 
 * for the shell to a directory specified in the command array.
 * @param command the tokenized command terminated by a NULL pointer.
 */
void cmd_cd(char ** command) {
    if(command[1] != NULL) {
        if(chdir(command[1])) {
            printf("Directory %s not found\n", command[1]);
        }
    } else {
        printf("cd: please provide a directory\n");
    }
}

/**
 * Prints the current working directory to the stream.
 */
void cmd_getcwd() {
    char * cwd = getcwd(NULL, MAX_PATH_LENGTH);
    printf("You are currently in %s\n", strrchr(cwd, '/'));
    free(cwd);
} 

/**
 * Displays the processes currently running in the background.
 * @param bg points to the list of processes currently running in the background.
 */
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

/**
 * Kills a background process that is currently running.
 * @param command the tokenized command terminated by a NULL pointer.
 * @param bg points to the list of processes currently running in the background.
 */
void cmd_kill(char ** command, struct bpid_list * bg) {
    int bpid = atoi(command[1]);
    if(remove_bp(bg, bpid)) {
        printf("Process %d has been killed\n", bpid);
        kill(bpid, SIGKILL);
    } else {
        printf("\033[0;31mBackground Process Error: Process %d not found\033[0m\n", bpid);
    }
}

/**
 * Prints whatever follows the echo command to the stream.
 * @param command the tokenized command terminated by a NULL pointer.
 */
void cmd_echo(char ** command) {
    if(command[1] == NULL) {
        printf("\n");
    }
    int i = 1;
    while(command[i+1] != NULL) {
        printf("%s ", command[i]);
        i++;
    }

    printf("%s\n", command[i]);
}

//****************************************************************************

//*****************EXTERNAL COMMANDS******************************************

/**
 * Determines whether then command is redirecting output to a file. 
 * Inserts a NULL pointer at the position of the file redirect token
 * in the command array.
 * @return the index of the filename in the command array if the command
 * is redirecting output, otherwise 0.
 */
int redirect_file(char ** command) {
    int i = 0;
    while(command[i] != NULL) {
        if(command[i][0] == '>' && command[i+1] != NULL) {
            command[i] = NULL;
            return i+1;
        }
        i++;
    } 
    return 0;
}

/**
 * Helper function that sets the output stream to a file.
 * @param filename pointer to string containing the name of the file to which output will be written.
 */
void set_output_stream(char * filename) {
    FILE * file = fopen(filename, "w");
    if(dup2(fileno(file), STDOUT_FILENO) == -1) {
        printf("\033[0;31mFile Redirect Error: File %s could not be written to\033[0m\n", filename);
    }
    fclose(file);
}

/**
 * Helper function to determine if a command is supposed to run 
 * in the background (i.e. the last item in the command list 
 * is an ampersand). 
 * 
 * The beauty of this function is that its return value serves 
 * two purposes: 
 * 1) it returns a value that the compiler interprets as true if 
 *      the command designates a background process and false if 
 *      not, and 
 * 2) the value it returns corresponds to the position of the 
 *      ampersand in the command list, so that any function 
 *      using it can set that position to a NULL pointer, 
 *      allowing for the command string to be fed into the 
 *      execv function as it normally would.
 * @param command the tokenized command terminated by a NULL pointer.
 * @return the index of the ampersand item in the command list if command 
 * designates a background process, otherwise 0.
 */
int cmd_is_bg(char ** command) {
    int i = 0;
    while(command[i] != NULL) { // find the end of the command list
        i++;
    }

    if(command[i-1][0] == '&') {    // Figure out if the item before the NULL
                                    // pointer is an ampersand.
        return i-1;                 // Return the position of the ampersand.
    }
    
    return 0;
}

/**
 * Runs a shell process or script off of a file.
 * @param command the tokenized command terminated by a NULL pointer.
 */
void cmd_extern(char ** command, struct bpid_list * bg) {
    int is_bg = cmd_is_bg(command);
    int pid = fork();
    if(pid == 0) { // child process
        if(is_bg) {
            command[is_bg] = NULL;
        }

        int is_redirect = redirect_file(command);
        if(is_redirect) {
            set_output_stream(command[is_redirect]);
        }

        if(execv(command[0], command) < 0) {
            fprintf(stdout, "\033[0;31mExternal Command Error: File \"%s\" not found\033[0m\n", command[0]);
        }
        exit(0);
    } else if(pid > 0) { // parent process
        if(is_bg) {
            if(add_bp(bg, pid, 0)) {
                printf("Created background process %d\n", pid);
            } else {
                printf("\033[0;31mBackground Process Error: Not enough space in background, killing process\033[0m\n");
                kill(pid, SIGKILL);
            }
        } else {
            waitpid(pid, 0, 0);
        }
    }
}

//*************************************************************************

/**
 * Takes in a tokenized command string terminated by a NULL pointer and the
 * list of background processes currently running, and decides which
 * command to execute.
 * @param command the tokenized command terminated by a NULL pointer
 * @param bg the list of processes running in the background
 */
void runcmd(char ** command, struct bpid_list * bg) {
    if(command[0] == NULL) return; // primary check to see if valid command

    if(!strcmp(command[0], "exit")) { // command is exit cmd
        cmd_exit(command, bg);
    } else if (!strcmp(command[0], "help")) { // command is help cmd
        cmd_help();
    } else if (!strcmp(command[0], "getcwd")) { // command is getcwd cmd
        cmd_getcwd();
    } else if (!strcmp(command[0], "cd")) { // command is cd cmd
        cmd_cd(command);
    } else if(!strcmp(command[0], "echo")) { // command is echo cmd
        cmd_echo(command);
    } else if (!strcmp(command[0], "jobs")) { // command is jobs cmd
        cmd_jobs(bg);
    } else if (!strcmp(command[0], "kill")) { // command is kill cmd
        cmd_kill(command, bg);
    } else if (command[0][0] == '/') { // command is external
        cmd_extern(command, bg);
    } else { // command is not found
        printf("\033[0;31mCommand not found. Enter \"help\" for a list of available commands\033[0m\n");
    }
}