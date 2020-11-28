/**
 * Header file for shell command set and command selection
 * function.
 */
#ifndef COMMANDS_H
#define COMMANDS_H

// The maximum length of the path to 
// be displayed as the current working 
// directory
#define MAX_PATH_LENGTH 256

// Function prototype for the command selection function
// that will be needed by the other programs.
// All other functions in commands.c are essentially protected 
// by this abstraction.
void runcmd(char ** command, struct bpid_list * bg);

#endif