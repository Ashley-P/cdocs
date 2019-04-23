#include <stdlib.h>
#include "ll.h"

// Function Prototypes
static inline void unlink_node(struct Node *node);



struct List *list_init() {
    struct List *ptr = malloc(sizeof(struct List));
    ptr->size = 0;
    ptr->head = NULL;
    ptr->tail = NULL;

    return ptr;
}

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

#if 0
void list_push_front(struct Node **head, struct Node *new_node) {
    new_node->next = *head;
    if (*head)
        (*head)->prev = new_node;
    *head = new_node;
}
#endif

void list_push_front(struct List *list, struct Node *node) {
    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    }

    list->size++;
}

#if 0
void list_push_back(struct Node *head, struct Node *new_node) {
     if (!head)
         head = new_node;
     else if (head->next)
         list_push_back(head->next, new_node);
    else {
        head->next = new_node;
        new_node->prev = head;
    }
}
#endif

void list_push_back(struct List *list, struct Node *node) {
    if (!list->tail) {
        list_push_front(list, node);
        return;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
        list->size++;
    }
}

#if 0
struct Node *list_pop_front(struct Node **head) {
    struct Node *rtn = *head;

    (*head)->next->prev = NULL;
    *head = (*head)->next;

    return rtn;
}
#endif

struct Node *list_pop_front(struct List *list) {
    struct Node *rtn;
    if (!list->head) {
        return NULL;
    } else {
        rtn = list->head;
        list->head = list->head->next;
        list->head->prev = NULL;
        list->size--;
        return rtn;
    }
}

#if 0
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
#endif

struct Node *list_pop_back(struct List *list) {
    struct Node *rtn;
    if (!list->tail) {
        return NULL;
    } else {
        rtn = list->tail;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
        list->size--;
        return rtn;
    }
}

// pops the node at the position
struct Node *list_pop_node(struct List *list, unsigned int pos) {
    struct Node *rtn;
    struct Node *srch; // Node used for searching
    if (pos == 0) {
        rtn = list_pop_front(list);
        list->size--;
        return rtn;
    } else if (pos == list->size) {
        rtn = list_pop_back(list);
        list->size--;
        return rtn;
    } else if (pos > list->size) {
        return NULL;
    } else {
        srch = list->head;
        for (int i = 0; i < pos; i++)
            srch = srch->next;
        unlink_node(srch);
        return srch;
    }
}

// -1 on failure
int list_pop_node2(struct List *list, struct Node *node) {
    struct Node *srch; // Node used for searching
    if (node == list->head) {
        list->head = list->head->next;
    } else if (node == list->tail) {
        list->tail = list->tail->prev;
    } else {
        srch = list->head;
        while (srch->next != NULL) {
            srch = srch->next;
            if (srch == node) {
                unlink_node(node);
                list->size--;
                return 1;
            }
        }
        return -1;
    }

    unlink_node(node);
    list->size--;
    return 1;
}

// Unlinks the node from the list
static inline void unlink_node(struct Node *node) {
    if (node->prev)
        node->prev->next = node->next;

    if (node->next)
        node->next->prev = node->prev;
}
