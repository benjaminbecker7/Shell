/*****
* Project 04: Shell
* COSC 208, Introduction to Computer Systems, Fall 2020
*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "background.h"
#include "commands.h"
#include "shell.h"

#define PROMPT "shell> "

/**
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

/**
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

void display_welcome() {
    printf("\033[1;36m");
    printf("--------------------------------------\n");
    printf("| Welcome to the Ben and Alex Shell! |\n");
    printf("|             (aka BASH)             |\n");
    printf("--------------------------------------\n");
    printf("\033[0m");
    printf("\033[1m");
    printf("For help with this shell, enter \"help\"\n");
    printf("\033[0m");
}

int main(int argc, char **argv) {
    system("reset");
    display_welcome();
    // main loop for the shell
    printf("%s ", PROMPT);
    fflush(stdout);  // Display the prompt immediately
    char buffer[1024];

    struct bpid_list * bg = init_background(); // malloc the background struct 

    while (fgets(buffer, 1024, stdin) != NULL) {
        manage_background(bg); // check background processes

        char **command = tokenize(buffer, " \t\n"); // tokenize input

        runcmd(command, bg); // run command

        free_tokens(command); // 
        
        printf("%s ", PROMPT);
        fflush(stdout);  // Display the prompt immediately
    }

    return EXIT_SUCCESS;
}
