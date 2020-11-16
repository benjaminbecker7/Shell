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

int runcmd(char ** command, struct bpid_list * bg) {
    if(!strcmp(command[0], "exit")) {
        free_tokens(command);
        exit(0);
    } else if (!strcmp(command[0], "jobs")) {
        if(bg->size > 0) {
            printf("Processes currently active: ");
            struct bpid_list_node * current = bg->head;
            while(current->next != NULL) {
                printf("%d, ", current->bpid);
                current = current->next;
            }
            printf("%d\n", current->bpid);
            return 1;
        } else if(bg->size == 0) {
            printf("No background processes currently active\n");
        }
    } else if (!strcmp(command[0], "kill")) {
        int bpid = atoi(command[1]);
        if(remove_bp(bg, bpid)) {
            kill(bpid, SIGKILL);
            return 1;
        } 
    } else if (command[0] != NULL) {
        
        int i = 0;
        while(command[i+1] != NULL) {
            i++;
        }

        if(command[i][0] == '&') {
            int bpid = fork();
            if(bpid == 0) {
                command[i] = NULL;
                add_bp(bg, bpid, (int) NULL);
                printf("Created background process %d", getpid());
                if(execv(command[0], command) < 0) {
                    fprintf(stderr, "File \"%s\" not found\n", command[0]);
                }
                exit((int) NULL);
            } else if(bpid > 0) {
                add_bp(bg, bpid, (int) NULL);
                return 1;
            }
        } else {
            int pid = fork();
            if(pid == 0) {
                if(execv(command[0], command) < 0) {
                    fprintf(stderr, "File \"%s\" not found\n", command[0]);
                }
                exit((int) NULL);
            } else if(pid > 0) {
                waitpid(pid, NULL, (int) NULL);
                return 1;
            }
        }
    } 
    

    return 0;
}

int main(int argc, char **argv) {
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
