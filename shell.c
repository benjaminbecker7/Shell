/*****
* Project 04: Shell
* COSC 208, Introduction to Computer Systems, Fall 2020
*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "background.h"

#define PROMPT "shell> "

/*
 * Break input string into an array of strings.
 * @param input the string to tokenize
 * @param delimiters the characters that delimite tokens
 * @return the array of strings with the last element of the array set to NULL
 */
char** tokenize(const char *input, const char *delimiters) {
    char *token = NULL;

    // make a copy of the input string, because strtok
    // likes to mangle strings.  
    char *input_copy = strdup(input);

    // find out exactly how many tokens we have
    int count = 0;
    for (token = strtok(input_copy, delimiters); token; 
            token = strtok(NULL, delimiters)) {
        count++ ;
    }
    free(input_copy);

    input_copy = strdup(input);

    // allocate the array of char *'s, with one additional
    char **array = (char **)malloc(sizeof(char *)*(count+1));
    int i = 0;
    for (token = strtok(input_copy, delimiters); token;
            token = strtok(NULL, delimiters)) {
        array[i] = strdup(token);
        i++;
    }
    array[i] = NULL;
    free(input_copy);
    return array;
}

/*
 * Free all memory used to store an array of tokens.
 * @param tokens the array of tokens to free
 */
void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

void cmd_exit(char ** command) {
    free_tokens(command);
    exit(0);
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
        printf("Created background process %d\n", bpid);
        add_bp(bg, bpid, 0);
    }
}

void runcmd(char ** command, struct bpid_list * bg) {
    if(command[0] == NULL) return; // primary check to see if valid command

    if(!strcmp(command[0], "exit")) { // command is an exit cmd
        cmd_exit(command);
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

int main(int argc, char **argv) {
    //clear();
    // main loop for the shell
    printf("%s", PROMPT);
    fflush(stdout);  // Display the prompt immediately
    char buffer[1024];
    struct bpid_list * bg = init_background();

    while (fgets(buffer, 1024, stdin) != NULL) {
        manage_background(bg);

        char **command = tokenize(buffer, " \t\n");

        runcmd(command, bg);

        free_tokens(command);

        printf("%s", PROMPT);
        fflush(stdout);  // Display the prompt immediately
    }

    return EXIT_SUCCESS;
}
