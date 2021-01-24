struct bitmap_manager {
	unsigned char *bitmap;
	unsigned int next_free;
};

unsigned int bitman_alloc(struct bitmap_manager *bm);
void bitman_free(struct bitmap_manager *bm, unsigned int pos);