#include <stdlib.h>
#include "ll.h"




struct Node *create_node(void *data) {
    struct Node *ptr = malloc(sizeof(struct Node));

    ptr->data = data;
    ptr->next = NULL;
    ptr->prev = NULL;

    return ptr;
}

// Doesn't delete the data
void delete_node(struct Node *node) {
    free(node);
}

void list_push_front(struct Node **head, struct Node *new_node) {
    new_node->next = *head;
    if (*head)
        (*head)->prev = new_node;
    *head = new_node;
}

void list_push_back(struct Node *head, struct Node *new_node) {
    if (head->next)
        list_push_back(head->next, new_node);
    else {
        head->next = new_node;
        new_node->prev = head;
    }
}

struct Node *list_pop_front(struct Node **head) {
    struct Node *rtn = *head;

    (*head)->next->prev = NULL;
    *head = (*head)->next;

    return rtn;
}

struct Node *list_pop_back(struct Node *head) {
    struct Node *rtn;

    if (head->next)
        rtn = list_pop_back(head->next);
    else {
        head->prev->next = NULL;
        return head;
    }

    return rtn;
}

// Unlinks the node from the list
void list_remove_node(struct Node *node) {
    if (node->prev)
        node->prev->next = node->next;

    if (node->next)
        node->next->prev = node->prev;
}
