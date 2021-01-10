#include <string.h>
#include <errno.h>

#define TEST_MEM(p) (if (p == NULL) printf("fatal error: "#p": %s\n", strerror(errno));)

#define OPT_ZIP 1
#define OPT_UNZIP 2

#define OPT_EOUTN 4