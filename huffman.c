#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "huffman.h"
#include "rlib.h"

int main(int argc, const char **argv) {
    int in_fd;
    int out_fd;
    unsigned int option;
    char opt;
    char *filename;
    char *output_name;

    filename = NULL;
    output_name = NULL;
    option = 0;
    opterr = 0;

    while ((opt = getopt(argc, (char *const *) argv, "z:u:o:")) != -1) {
        switch (opt) {
            case 'z':
                option = OPT_ZIP;
                if (filename != NULL) {
                    printf("fatal error: only one input file. \n");
                    return 1;
                }

                filename = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(filename);

                strcpy(filename, optarg);
                break;
            
            case 'u':
                option = OPT_UNZIP;
                if (filename != NULL) {
                    printf("fatal error: only one input file. \n");
                    return 1;
                }

                filename = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(filename);

                strcpy(filename, optarg);
                break;

            case 'o':
                output_name = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(output_name);

                strcpy(output_name, optarg);
                break;

            default:
                printf("fatal error: unknow command-line option: %c\n", opt);
                return 1;
        }
    }

    if (option != OPT_ZIP && option != OPT_UNZIP) {
        printf("fatal error: you must choose one option. \n");
        return 1;
    }

    if (output_name == NULL) {
        output_name = (char *) malloc(strlen("output.riz") + 1);
        strcpy(output_name, "output.riz");
    }

    in_fd = open(filename, O_RDONLY);
    out_fd = open(output_name, O_RDWR | O_CREAT);

    if (in_fd < 0) {
        printf("fatal error: %s: %s\n", filename, strerror(errno));
        return 1;
    }

    if (out_fd < 0) {
        close(in_fd);
        printf("fatal error: %s: %s\n", output_name, strerror(errno));
        return 1;
    }

    if (option == OPT_ZIP) {
        huffman_zip(in_fd, out_fd);
    }

    close(in_fd);
    close(out_fd);

    return 0;
}


/*
    sort list. (pop)
    1. read two count. 
    2. if (count_bigger < count_smaller):
        swap;
*/
void sort(unsigned int *buffer, int count) {
    int i;
    int j;
    unsigned int tmp;

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (buffer[j] > buffer[j+1]) {
                tmp = buffer[j];
                buffer[j] = buffer[j+1];
                buffer[j+1] = tmp;
            }
        }
    }
}

/*
    get all words in fd. 
    STEPS: 
        1. read size to word_buffer. 
        2. if word_buffer in all_words: 
            all_words.count++
        3. else:
            add to word_buffer all_words
        
        4. sort words (pop)
*/
int get_words(int size, int fd) {
    unsigned int byte_count;
    unsigned int word_count;
    
    unsigned char *buffer;
    unsigned int *count_buffer;
    unsigned char *all_words;

    int i;
    int word_read;

    byte_count = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    
    if (byte_count % size == 0) {
        word_count = byte_count / size;
    }

    else {
        word_count = (byte_count / size) + 1;
    }

    buffer = malloc(size);
    all_words = malloc(size * word_count);
    count_buffer = malloc(word_count * sizeof(unsigned int));

    word_read = 0;

while_continue:
    while (read(fd, buffer, size) != 0) {
        for (i = 0; i < word_read; i++) {
            if (strcmp(buffer, (char *) &all_words[word_read * size]) == 0) {
                printf("have word %s, count %d\n", (char *) &all_words[word_read * size], count_buffer[i]);
                count_buffer[i]++;
                word_read++;
                goto while_continue;
            }
        }
        
        
        strcpy((char *) &all_words[word_read * size], buffer);
        printf("new word %s\n", (char *) &all_words[word_read * size]);
        count_buffer[word_read] = 0;
        word_read++;
        goto while_continue;
    }
}

/*
    zip in_fd to out_fd. 
    STEPS:
        1. get all word exists. 
        2. add words to a BST. 
        3. write BST to out_fd. 
        4. traverse BST, and write to out_fd. 
*/

int huffman_zip(int in_fd, int out_fd) {
    get_words(5, in_fd);

    return 0;
}