#ifndef COMMANDS_H
#define COMMANDS_H

// The maximum length of the path to 
// be displayed as the current working 
// directory
#define MAX_PATH_LENGTH 256

void runcmd(char ** command, struct bpid_list * bg);

#endif