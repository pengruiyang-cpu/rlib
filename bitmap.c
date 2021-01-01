#include<stdlib.h>

unsigned char *bitmap_init(unsigned int size) {
	unsigned char *bitmap = malloc(size);
	return bitmap;
}


void bitmap_set(unsigned char *bitmap, unsigned int pos, unsigned char value) {
	unsigned char mask = 0x80 >> (pos & 0x7);
	if (value) {
		bitmap[pos >> 3] |= mask;
	}
	else {
		bitmap[pos >> 3] &= ~mask;
	}
}

char bitmap_read(unsigned char *bitmap, unsigned int pos) {
	unsigned char mask = 0x80 >> (pos & 0x7);

	return (mask & bitmap[pos >> 3]) == mask ? 1 : 0;
}

