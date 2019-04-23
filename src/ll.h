#ifndef LL_H
#define LL_H


struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
};

struct List {
    unsigned int size;
    struct Node *head;
    struct Node *tail;
};

// Extern Functions
struct List *list_init();
struct Node *create_node(void *data);
void delete_node(struct Node *node);
void list_push_front(struct List *list, struct Node *node);
void list_push_back(struct List *list, struct Node *node);
struct Node *list_pop_front(struct List *list);
struct Node *list_pop_back(struct List *list);
struct Node *list_pop_node(struct List *list, unsigned int pos);
int list_pop_node2(struct List *list, struct Node *node);

#endif
