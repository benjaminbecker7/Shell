#ifndef SHELL_H
#define SHELL_H

// The maximum length of the path to 
// be displayed as the current working 
// directory
#define MAX_PATH_LENGTH 256

// current working directory
// TODO: Initialize and get cwd
char * cwd;

void free_tokens(char **tokens);

#endif