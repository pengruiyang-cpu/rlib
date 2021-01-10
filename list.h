struct list_node {
    char used;
    unsigned int value;
    void *pointer;
    struct list_node *next;
};

void list_init(struct list_node *head);
struct list_node *get_last(struct list_node *head);
struct list_node *list_add_last(struct list_node *head, unsigned int value, void *pointer);
struct list_node *list_read_first(struct list_node *head);
struct list_node *list_rm_first(struct list_node *head);