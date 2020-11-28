/**
 * Contains all of the functions that manage the background processes
 * that can be run in the shell.
 */
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include "background.h"

/**
 * Initializes and mallocs the list of background processes.
 * @return the initialized list of background processes with size 0.
 */
struct bpid_list * init_background() {
    struct bpid_list * bg = malloc(sizeof(struct bpid_list));
    bg->head = NULL;
    bg->size = 0;
    return bg;
}

/**
 * Adds a background process to the list of background processes.
 * @param bg points to the list of background processes currently running.
 * @param bpid the process ID of the background process that will be added.
 * @param exitstatus the expected exit status that the new background process should return.
 * @return 1 if the background process has been successfully added, 0 if not.
 */
int add_bp(struct bpid_list * bg, int bpid, int exitstatus) {
    if(bg->size + 1 < MAX_BACKGROUND) { // check if room
        struct bpid_list_node * new = malloc(sizeof(struct bpid_list_node));
        new->bpid = bpid;
        new->exitstatus = malloc(sizeof(int));
        *(new->exitstatus) = exitstatus;
        new->next = NULL;
        if(bg->head == NULL) { // head is NULL, set head to point to new process
            bg->head = new;
        } else { // head is not NULL, add new node to back of list.
            struct bpid_list_node * current = bg->head;
            while(current->next != NULL) {
                current = current->next;
            }
            current->next = new;
        }
        bg->size++;
        return 1;
    }
    return 0;
}

/**
 * Removes a background process from the list of processes currently running in the
 * background of the shell.
 * @param bg points to the list of processes running in the background.
 * @param bpid the process ID of the background process to be removed.
 * @return 1 if the process was successfully removed, 0 if not.
 */
int remove_bp(struct bpid_list * bg, int bpid) {
    if(bg->head == NULL) { // head is NULL, bg is therefore empty. Do nothing and return 0
        return 0;
    } else if(bg->head->bpid == bpid) { // head has target bpid, remove the head node and return 1
        struct bpid_list_node * target = bg->head;
        bg->head = bg->head->next;
        free(target->exitstatus);
        free(target);
        target = NULL;
        bg->size--;
        return 1;
    } else { // target bp might be after head
        struct bpid_list_node * current = bg->head;
        while(current->next != NULL && current->next->bpid != bpid) {   // Start cycling with the head and 
                                                                        // keep cycling until the next node
                                                                        // either contiains the target process
                                                                        // or is NULL
            current = current->next;
        }
        if(current->next != NULL) { // If the next node is not NULL, then the target process must have been found.
                                    // Therefore, we must remove that node containing the target process and return 1.
            struct bpid_list_node * target = current->next;
            current->next = current->next->next;
            free(target);
            target = NULL;
            bg->size--;
            return 1;
        }
        return 0;   // Target process was not found; return 0.
    }
}

/**
 * Frees the list of background processes.
 * Kills each background process in the list before freeing it.
 * @param bg the list of background processes to be freed.
 */
void free_background(struct bpid_list * bg) {
    while(bg->size > 0) {
        kill(bg->head->bpid, SIGKILL);
        remove_bp(bg, bg->head->bpid);
    }
    free(bg);
    bg = NULL;
}

/**
 * Goes through list of background processes to see if any have finished running.
 * Removes any terminated processes from the list.
 * @param bg points to the list of processes running in the background of the shell.
 */
void manage_background(struct bpid_list * bg) {
    struct bpid_list_node * current = bg->head;
    while(current != NULL) {
        int status = waitpid(current->bpid, current->exitstatus, WNOHANG);
        if(status > 0) { // Process finished successfully; remove and go to next
            printf("%d finished with exit code %d\n", status, *(current->exitstatus));
            struct bpid_list_node * next = current->next;
            remove_bp(bg, current->bpid);
            current = next;
        } else if(status < 0) { // Process finished with error; print to stderr, 
                                // remove process, and move to next process.
            fprintf(stderr, "Process %d has encountered an error\n", current->bpid);
            struct bpid_list_node * next = current->next;
            remove_bp(bg, current->bpid);
            current = next;
        } else { // Process is still running; go to next
            current = current->next;
        }
    }
}