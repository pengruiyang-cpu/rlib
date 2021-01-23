#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rlib.h"


/*
	find a new bit in bm->bitmap. 

	STEPS: 
		1. save free position. 
		2. find a new free position. 
		3. save new free position. 
*/
unsigned int bitman_alloc(struct bitmap_manager *bm) {
	unsigned int p = bm->next_free;
	bitmap_set(bm->bitmap, bm->next_free, 1);

	for (; bitmap_read(bm->bitmap, bm->next_free) == 1; bm->next_free++);

	return p;
}


/*
	free a bit. 

	STEPS: 
		1. free bit pos. 
		2. if pos is lowwer than bm->next_free
			2a. set bm->next_free = pos. 
*/
void bitman_free(struct bitmap_manager *bm, unsigned int pos) {
	bitmap_set(bm->bitmap, pos, 0);

	if (pos < bm->next_free) {
		bm->next_free = pos;
	}
	return;
}
