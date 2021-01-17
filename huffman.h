#include <string.h>
#include <errno.h>

#define TEST_MEM(p) if (p == NULL) printf("fatal error: "#p": %s\n", strerror(errno));

#define OPT_ZIP 1
#define OPT_UNZIP 2

int huffman_zip(int in_fd, int out_fd);

struct cs_entry {
    unsigned int c;
    char *s;
};