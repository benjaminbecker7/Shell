/**
 * Header file for the background processes linked list and functions
 */
#ifndef BACKGROUND_H
#define BACKGROUND_H

#define MAX_BACKGROUND 50   // Maximum amount of background processes that
                            // can be running at any given time.

/**
 * Struct representing individual process running in the background of
 * the shell.
 * bpid is the id of the background process.
 * exitstatus points to the integer that is supposed to be the exit status
 * of the background process.
 * next points to the next bpid_list_node in the bpid list.
 */
struct bpid_list_node {
    int bpid;
    int * exitstatus;
    struct bpid_list_node * next;
};

/**
 * Struct representing the list of processes running in the background
 * of the shell.
 * head points to the node at the head of the list.
 * size stores the size of the bpid list.
 */
struct bpid_list {
    struct bpid_list_node * head;
    int size;
};

// Background function prototypes needed for other functions.
void free_background(struct bpid_list * bg);
struct bpid_list * init_background();
int add_bp(struct bpid_list * bg, int bpid, int exitstatus);
int remove_bp(struct bpid_list * bg, int bpid);
void manage_background(struct bpid_list * bg);

#endif