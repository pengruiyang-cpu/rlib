#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "bstree.h"



struct bst *bst_init(void) {
	/* don't think more! only 8+8+8+8+4+1 = 29B (all pointers) */
	struct bst *bstree = malloc(sizeof(struct bst));
	
	memset(bstree, 0, sizeof(struct bst));
	
	return bstree;
}

void *bst_search(struct bst *bstree, unsigned int value) {
	struct bst *bstree_tmp;

	bstree_tmp = bstree;
	
	/* if value > this_value, right; else left */
	for (;;) {
		if (bstree_tmp->flag == 0)  {
			return bstree_tmp->value_ptr;
		}

		if (value > bstree_tmp->value) {
			bstree_tmp = bstree->child_right;
			if (bstree_tmp == NULL) return NULL;
		}
		
		else if (value < bstree_tmp->value) {
			bstree_tmp = bstree->child_left;
			if (bstree_tmp == NULL) return NULL;
		}

		else {
			return bstree_tmp->value_ptr;
		}
	}
}

int bst_write(struct bst *bstree, unsigned int value, void *value_ptr) {
	struct bst *bstree_tmp;
	bstree_tmp = bstree;
	
	for (;;) {
		if (bstree_tmp->flag == 0)  {
			bstree_tmp->value = value;
			bstree_tmp->value_ptr = value_ptr;
			bstree_tmp->flag = 1;
			return 0;
		}

		if (value > bstree_tmp->value) {
			if (bstree_tmp->child_right == NULL) {
				/* insert it! */
				bstree_tmp->child_right = bst_init();
				bstree_tmp = bstree_tmp->child_right;
				bstree_tmp->value = value;
				bstree_tmp->value_ptr = value_ptr;
				return 0;
			}

			else {
				bstree_tmp = bstree_tmp->child_right;
			}
		}
		
		else if (value < bstree_tmp->value) {
			if (bstree_tmp->child_left == NULL) {
				/* insert it! */
				bstree_tmp->child_left = bst_init();
				bstree_tmp = bstree_tmp->child_left;
				bstree_tmp->value = value;
				bstree_tmp->value_ptr = value_ptr;
				return 0;
			}

			else {
				bstree_tmp = bstree_tmp->child_left;
			}
		}

		else {
			/* value is already in tree */
			return -1;
		}
	}
}
