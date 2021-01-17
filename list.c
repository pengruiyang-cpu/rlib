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

struct list_node *list_read(struct list_node *head, unsigned int pos) {
    struct list_node *tmp;
    int i;

    for (i = 0, tmp = head; i <= pos && tmp != NULL; i++, tmp = tmp->next);
    return tmp;
}

struct list_node *list_read_last(struct list_node *head) {
    return get_end(head);
}

unsigned int list_length(struct list_node *head) {
    struct list_node *tmp = NULL;
    int i;

    for (i = 0, tmp = head; tmp->next != NULL && tmp->next->used == 1; tmp = tmp->next, i++);
    return i;
}


struct list_node *list_rm_last(struct list_node *head) {
    int i;
    int len = list_length(head);
    struct list_node *tmp;

    for (i = 0, tmp = head; i < len - 1; i++, tmp = tmp->next);
    tmp->next = NULL;
    return tmp;
}