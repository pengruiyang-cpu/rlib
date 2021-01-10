#include <stdlib.h>
#include <stddef.h>
#include "list.h"

void list_init(struct list_node *head) {
    head->used = 1;
    head->value = 0;
    head->pointer = NULL;
    head->next = NULL;
       
    return;
}

struct list_node *get_end(struct list_node *head) {
    struct list_node *tmp = NULL;
    for (tmp = head; tmp->next != NULL && tmp->next->used == 1; tmp = tmp->next);
    return tmp;
}

struct list_node *list_add_last(struct list_node *head, unsigned int value, void *pointer) {
    struct list_node *last;
    last = get_end(head);

    last->next = (struct list_node *) malloc(sizeof(struct list_node));

    last->next->used = 1;
    last->next->value = value;
    last->next->pointer = pointer;
    last->next->next = NULL;
    return last->next;
}

struct list_node *list_read_first(struct list_node *head) {
    /* head.used = 1, but never uses. */
    return head->next;
}

struct list_node *list_rm_first(struct list_node *head) {
    /* head.used = 1, but never uses. */
    head->next = head->next->next;
    return head;
}
