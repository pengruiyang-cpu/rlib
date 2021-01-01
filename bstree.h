struct bst {
	struct bst *parent;
	struct bst *child_left;
	struct bst *child_right;
	unsigned int value;
	void *value_ptr;
	char flag;
};


extern struct bst *bst_init(void);
extern void *bst_search(struct bst *bstree, unsigned int value);
extern int bst_write(struct bst *bstree, unsigned int value, void *value_ptr);
