#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

int main(int argc, const char **argv) {
	int i_fd;
	char *i_file;
	size_t i_size;

	int o_fd;
	char *o_file;
	size_t o_size;

	size_t size_wrote;

	char buffer[4096 + 1] = {0};

	if (argc < 3) {
		printf("panic: too few arguments\n");
		printf("use 'fcp source-file dest-file'\n");
		return 1;
	}

	i_file = malloc(
		strlen(argv[1])
	);

	if (i_file == NULL) {
		printf("panic: memory alloc failed: %s\n", 
			strerror(errno)
		);
		return 1;
	}

	o_file = malloc(sizeof(strlen(argv[2])));

	if (o_file == NULL) {
		printf("panic: memory alloc failed: %s\n", 
			strerror(errno)
		);
		free(i_file);
		return 1;
	}

	strcpy(i_file, argv[1]);
	strcpy(o_file, argv[2]);

	i_fd = open(i_file, O_RDONLY);

	if (i_fd == -1) {
		printf("panic: %s: %s\n", i_file, 
			strerror(errno)
		);
		free(i_file);
		free(o_file);
		return 1;
	}

	o_fd = open(o_file, O_WRONLY | O_CREAT);

	if (o_fd == -1) {
		printf("panic: %s: %s\n", o_file, 
			strerror(errno)
		);
		free(i_file);
		free(o_file);
		close(i_fd);
		return 1;
	}

	i_size = lseek(i_fd, 0, SEEK_END);
	lseek(i_fd, 0, SEEK_SET);

	printf("Loading: \n");

	for (size_wrote = 0; ; size_wrote += 4096) {
		read(i_fd, buffer, 4096);
		write(o_fd, buffer, 4096);

		size_wrote += 4096;
		if (size_wrote > i_size) {
			break;
		}
		printf("\r%lf %ldMB / %ldMB", (double) size_wrote / i_size, size_wrote, i_size);
	}

	free(i_file);
	free(o_file);
	close(i_fd);
	close(o_fd);

	return 0;
}

