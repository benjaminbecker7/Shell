#ifndef BACKGROUND_H
#define BACKGROUND_H

#define MAX_BACKGROUND 50

struct bpid_list_node {
    int bpid;
    int * exitstatus;
    struct bpid_list_node * next;
};

struct bpid_list {
    struct bpid_list_node * head;
    int size;
};

void free_background(struct bpid_list * bg);
struct bpid_list * init_background();
int add_bp(struct bpid_list * bg, int bpid, int exitstatus);
int remove_bp(struct bpid_list * bg, int bpid);
void manage_background(struct bpid_list * bg);

#endif