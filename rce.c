#include <stdio.h>

unsigned int encodeb(unsigned int k, unsigned int s) {
	return (s + k) ^ k ^ !k;
}

unsigned int decodeb(unsigned int k, unsigned int r) {
	return (r ^ !k ^ k) - k;
}

unsigned int encode(unsigned int k, unsigned int s) {
	return encodeb(encodeb(k, k), s);
}

unsigned int decode(unsigned int k, unsigned int r) {
	return decodeb(encodeb(k, k), r);
}


int main(void) {
	unsigned int r = encode(20090427, 12345);
	printf("source: %ud\n", r);
	printf("source: %ux\n", decode(20090427, r));
}

