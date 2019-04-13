#ifndef LL_H
#define LL_H


struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
};

// Extern Functions
struct Node *create_node(void *data);
void delete_node(struct Node *node);
void list_push_front(struct Node **head, struct Node *new_node);
void list_push_back(struct Node *head, struct Node *new_node);
struct Node *list_pop_front(struct Node **head);
struct Node *list_pop_back(struct Node *head);
void list_remove_node(struct Node *node);

#endif
