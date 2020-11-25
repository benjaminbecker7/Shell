#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include "background.h"

struct bpid_list * init_background() {
    struct bpid_list * bg = malloc(sizeof(struct bpid_list));
    bg->head = NULL;
    bg->size = 0;
    return bg;
}

int add_bp(struct bpid_list * bg, int bpid, int exitstatus) {
    if(bg->size + 1 < MAX_BACKGROUND) { // check if room
        struct bpid_list_node * new = malloc(sizeof(struct bpid_list_node));
        new->bpid = bpid;
        new->exitstatus = malloc(sizeof(int));
        *(new->exitstatus) = exitstatus;
        new->next = NULL;
        if(bg->head == NULL) {
            bg->head = new;
        } else {
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

int remove_bp(struct bpid_list * bg, int bpid) {
    if(bg->head == NULL) {
        return 0;
    } else if(bg->head->bpid == bpid) {
        struct bpid_list_node * target = bg->head;
        bg->head = bg->head->next;
        free(target);
        target = NULL;
        bg->size--;
        return 1;
    } else {
        struct bpid_list_node * current = bg->head;
        while(current->next != NULL && current->next->bpid != bpid) {
            current = current->next;
        }
        if(current->next != NULL) {
            struct bpid_list_node * target = current->next;
            current->next = current->next->next;
            free(target);
            target = NULL;
            bg->size--;
            return 1;
        }
        return 0;
    }
}

void free_background(struct bpid_list * bg) {
    while(bg->size > 0) {
        kill(bg->head->bpid, SIGKILL);
        remove_bp(bg, bg->head->bpid);
    }
    free(bg);
    bg = NULL;
}

void manage_background(struct bpid_list * bg) {
    struct bpid_list_node * current = bg->head;
    while(current != NULL) {
        //printf("bg process exists\n"); // test to see if there is still a bg process after kill
        int status = waitpid(current->bpid, current->exitstatus, WNOHANG);
        if(status > 0) {
            struct bpid_list_node * next = current->next;
            printf("%d finished with exit code %d\n", status, *(current->exitstatus));
            remove_bp(bg, current->bpid);
            current = next;
        } else if(status < 0) {
            fprintf(stderr, "Process %d has encountered an error\n", current->bpid);
        } else {
            current = current->next;
        }
    }
}