#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "huffman.h"

int main(int argc, const char **argv) {
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
                if (filename == NULL) {
                    printf("fatal error: only one input file. \n");
                    return 1;
                }

                filename = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(filename);

                strcpy(filename, optarg);
                break;
            
            case 'u':
                option = OPT_UNZIP;
                if (filename == NULL) {
                    printf("fatal error: only one input file. \n");
                    return 1;
                }

                filename = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(filename);

                strcpy(filename, optarg);
                break;

            case 'o':
                option |= OPT_EOUTN;
                output_name = (char *) malloc(strlen(optarg) + 1);

                TEST_MEM(output_name);

                strcpy(output_name, optarg);
                break;

            default:
                printf("fatal error: unknow command-line option: %c\n", opt);
                return 1;
        }
    }

    if (option != OPT_ZIP || option != OPT_UNZIP) {
        printf("fatal error: you most choose one option. \n");
        return 1;
    }

    if (output_name == NULL) {
        output_name = (char *) malloc(strlen(filename));
        strcpy(output_name, filename);
    }


}


/*
    get all words in fd. 
    STEPS: 
        1. read size to word_buffer. 
        2. if buffer in all_words (list): 
            all_words.count++
        3. else:
            add to word_buffer all_words
*/
int get_words(int size, int fd) {
    char *word_buffer = (char *) malloc(size);

    while (read(fd, word_buffer, size) != 0) {
        
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

}