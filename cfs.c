#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

/* rlib.h and other file is at https://github.com/pengruiyang-cpu/rlib.git */
#include "rlib.h"
#include "cfs.h"

#define FORMAT 1
#define CREATE 2
#define WRITE 3
#define READ 4

#define DEBUG

#ifdef DEBUG
int main(int argc, const char **argv) {
    int opr;
    int fd;
    char filename[1024] = {0};
    char option;
    char name_fs[1024] = {0};

    while ((option = getopt(argc, argv, "i:fcwrn")) != -1) {
        switch (option) {
            case 'i':
                strncpy(filename, optarg, 1024);
                break;

            case 'f':
                opr = FORMAT;
                break;

            case 'c':
                opr = CREATE;
                break;

            case 'w':
                opr = WRITE;
                break;

            case 'r':
                opr = READ;
                break;

            case 'n':
                strncpy(name_fs, optarg, 1024);
                break;

            default:
                printf("error: unknow command-line option '-%c'\n", option);
                return 1;
        }
    }

    if (filename[0] == '\0') {
        printf("fatal error: no input file\n");
        return 1;
    }

    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("fatal error: %s: %s\n", filename, strerror(errno));
        return 1;
    }

    if (opr == FORMAT) {
        cfs_format(fd);
        return 0;
    }

    if (opr == CREATE) {
        if (name_fs[0] == '\0') {
            cfs_create(cfs.root, name_fs,
                                IMD_AP_ROOT_RD | 
                                IMD_AP_ROOT_WR | 
                                IMD_AP_ROOT_XR | 
                                IMD_AP_USER_RD | 
                                IMD_AP_USER_WR | 
                                IMD_AP_USER_XR | 
                                IMD_FT_REGFILE
            );
        }

        else {
            printf("fatal error: no file name\n");
            return 1;
        }
        return 0;
    }

    if (opr == WRITE) {
        if (name_fs[0] == '\0') {

        }
    }

    cfs_writeback(fd);

    return 0;
}

#endif

/*
    write disk types to disk fd
    STEPS: 
        1. write super_block
        2. write block_bitmap
        3. init dir root
        4. write root
*/
int cfs_format(int fd) {
    __off_t disk_size;
    disk_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    unsigned int block_count = disk_size / 4096;

    memset(&superblock_d, 0, sizeof(struct super_block_d));
    memset(&cfs, 0, sizeof(struct cfs_m));

    superblock_d.mbr[510] = 0x55;
    superblock_d.mbr[511] = 0xaa;

    superblock_d.magic = SUPERBLOCK_MAGIC;

    superblock_d.block_count = block_count;

    superblock_d.block_bitmap = /* superblock */ 1 * 4096 / 4096;

    superblock_d.root_dir = 4096 / 4096 + ALIGNUP_4096(block_count / 8) / 4096;

    memset(&(superblock_d.reserved), 0, 3568);

    superblock_m.sb_disk = &superblock_d;
    cfs.superblock = &superblock_m;

    cfs.block_bitmap = malloc(ALIGNUP_4096(block_count / 8));
    if (cfs.block_bitmap == NULL) {
        return 1;
    }

    memset(cfs.block_bitmap, 0, ALIGNUP_4096(block_count / 8));

    cfs.root = malloc(sizeof(struct dir_d));
    if (cfs.root == NULL) {
        free(cfs.block_bitmap);
        return 1;
    }

    memset(cfs.root, 0, sizeof(struct dir_d));
    memset(cfs.root->inode_bitmap, 0, 4096 / 8);
    memset(cfs.root->inodes, 0, sizeof(struct inode_d) * 4096);
    memset(cfs.root->names, 0, 128 * 4096);

    /* init the root dir */
#define root cfs.root
#define ROOT_START_BLOCK /* superblock */ 4096 / 4096 + /* block bitmamp */ ALIGNUP_4096(block_count / 8) / 4096

    char filename[128] = {"."};

    cfs_create(root, filename, 
                                IMD_AP_ROOT_RD | 
                                IMD_AP_ROOT_WR | 
                                IMD_AP_ROOT_XR | 
                                IMD_AP_USER_RD | 
                                IMD_AP_USER_WR | 
                                IMD_AP_USER_XR | 
                                IMD_FT_FOLDER | 
                                IMD_FT_POINTER
    );

    root->inodes[0].block_count = ROOT_START_BLOCK;


    filename[1] = '.';
    cfs_create(root, filename, 
                                IMD_AP_ROOT_RD | 
                                IMD_AP_ROOT_WR | 
                                IMD_AP_ROOT_XR | 
                                IMD_AP_USER_RD | 
                                IMD_AP_USER_WR | 
                                IMD_AP_USER_XR | 
                                IMD_FT_FOLDER | 
                                IMD_FT_POINTER
    );

    root->inodes[1].block_count = ROOT_START_BLOCK;
    
    superblock_m.block_bitmap = cfs.block_bitmap;
    superblock_m.root_dir = root;
#define BLOCK_USED 1 /* superblock */ + ALIGNUP_4096(block_count / 8) / 4096 + sizeof(struct dir_d) / 4096

    int i;
    for (i = 1; i < BLOCK_USED; i++) {
        bitmap_set(cfs.block_bitmap, i - 1, 1);
    }

    try(write(fd, &superblock_d, sizeof(struct super_block_d)));
    try(write(fd, cfs.block_bitmap, ALIGNUP_4096(block_count / 8)));
    try(write(fd, root, sizeof(struct dir_d)));

    superblock_m.root_dir = root;

    return 0;

#undef ROOT_START_BLOCK
#undef root
}


/*
    read from disk. 
    1. read superblock
    2. read block bitmap
    3. read root-dir
*/
int cfs_init(int fd) {

    try(read(fd, &(superblock_d.mbr), 512));
    try(read(fd, &(superblock_d.magic), sizeof(unsigned int)));

    if (superblock_d.magic != SUPERBLOCK_MAGIC) {
        printf("warning: magic number isn't %x\n", SUPERBLOCK_MAGIC);
    }
    try(read(fd, &(superblock_d.block_count), sizeof(unsigned int)));
    try(read(fd, &(superblock_d.block_bitmap), sizeof(unsigned int)));
    try(read(fd, &(superblock_d.root_dir), sizeof(unsigned int)));
    try(read(fd, &(superblock_d.reserved), 3568));

    superblock_m.sb_disk = &superblock_d;

    cfs.block_bitmap = malloc(ALIGNUP_4096(superblock_d.block_count / 8));
    if (cfs.block_bitmap == NULL) {
        return 1;
    }

    try(read(fd, cfs.block_bitmap, ALIGNUP_4096(superblock_d.block_count / 8)));

#define root cfs.root
    root = malloc(sizeof(struct dir_d));
    if (root == NULL) {
        return 1;
    }

    try(read(fd, root->inode_bitmap, 4096 / 8));
    try(read(fd, root->inodes, sizeof(struct inode_d) * 4096));
    try(read(fd, root->names, 128 * 4096));

    superblock_m.root_dir = root;

    return 0;
#undef root
}

int cfs_writeback(int fd) {
    lseek(fd, 0, SEEK_SET);
    try(write(fd, &(superblock_d.mbr), 512));
    try(write(fd, &(superblock_d.magic), sizeof(unsigned int)));
    try(write(fd, &(superblock_d.block_count), sizeof(unsigned int)));
    try(write(fd, &(superblock_d.block_bitmap), sizeof(unsigned int)));
    try(write(fd, &(superblock_d.root_dir), sizeof(unsigned int)));
    try(write(fd, &(superblock_d.reserved), 3568));

    try(write(fd, cfs.block_bitmap, ALIGNUP_4096(superblock_d.block_count / 8)));

#define root cfs.root

    try(write(fd, root->inode_bitmap, 4096 / 8));
    try(write(fd, root->inodes, sizeof(struct inode_d) * 4096));
    try(write(fd, root->names, 128 * 4096));

    return 0;

#undef root
}

int cfs_ls(struct dir_d *dir) {
    int i;

    for (i = 0; i < 4096; i++) {
        if (bitmap_read(dir->inode_bitmap, i) == 1) {
            printf("inode %d, file name: %s\n", i, dir->names + i * 4096);
        }
    }
    return i;
}


unsigned int alloc_block(void) {
    int i;
    for (i = 0; bitmap_read(cfs.block_bitmap, i) == 1; i++);
    bitmap_set(cfs.block_bitmap, i, 1);
    return i;
}

/*
    create a new file in dir. 
    STEPS: 
        1. find a free inode
        2. write file name and mode. 
*/
struct inode_d *cfs_create(struct dir_d *dir, char *filename, unsigned short mode) {
    int inode;
    for (inode = 0; inode < 4096 && bitmap_read(dir->inode_bitmap, inode) == 1; inode++);

    bitmap_set(dir->inode_bitmap, inode, 1);

    dir->inodes[inode].mode = mode;
    dir->inodes[inode].inode = inode;
    memcpy(dir->names + inode * 4096, filename, 128 - 1);

    return &(dir->inodes[inode]);
}

int read_block(int fd, void *buffer, unsigned int size, unsigned int block) {
    __off_t now_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, block * 4096, SEEK_SET);
    try(read(fd, buffer, 4096));
    lseek(fd, now_pos, SEEK_SET);

    printf("read block: %p\n", buffer);

    int i;
    for (i = 0; i < 1024; i++) printf("%x", ((unsigned int *) &(buffer)) [i]);

    putchar('\n');
}

int write_block(int fd, void *buffer, unsigned int size, unsigned int block) {
    printf("write block: %p\n", buffer);
    int i;
    for (i = 0; i < 1024; i++) printf("%x", ((unsigned int *) &(buffer)) [i]);
    putchar('\n');

    __off_t now_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, block * 4096, SEEK_SET);
    try(write(fd, buffer, size));
    lseek(fd, now_pos, SEEK_SET);
}


/*
    write buffer to file. 

    STEPS: 
        1. how many blocks we need? 
        2. write to 1st block

    WARNING: 
        1. ABS(buffer.size / 4096) = buffer.size / 4096
*/
unsigned int cfs_write(int fd, struct inode_d *inode, char *buffer, unsigned int size) {
    unsigned int block_need = ALIGNUP_4096(size) / 4096;
    unsigned int block_wrote = 0;
    
    unsigned int blocks_1st[1024];
    unsigned int blocks_2nd[1024];
    unsigned int blocks_3rd[1024];

#define c_1st block_wrote
#define c_1st_c block_wrote
    unsigned int c_2nd = 0;
    unsigned int c_2nd_c = 0;

    unsigned int c_3rd = 0;
    unsigned int c_3rd_c = 0;

    for (block_wrote = 0; block_wrote < block_need; block_wrote++) {
        if (block_wrote < 1024) {
            if (block_wrote == 0) {
                inode->block_ptr_1 = alloc_block();
                memset(blocks_1st, 0, 1024 * 4);
            }

            blocks_1st[c_1st_c] = alloc_block();

            printf("write it: %p\n", blocks_1st);
            int i;
            for (i = 0; i < 1024; i++) printf("%x", blocks_1st[i]);
            putchar('\n');


            write_block(fd, (void *) &blocks_1st, 1024 * 4, inode->block_ptr_1);
            printf("write to %d, %p\n", inode->block_ptr_1, blocks_1st);
            write_block(fd, (char *) (buffer + block_wrote * 4096), 1024 * 4, blocks_1st[block_wrote]);
            printf("write to %d\n", blocks_1st[block_wrote], (buffer + block_wrote * 4096));
        }

        else if (block_wrote >= 1024 < 1048576) {
            if (block_wrote == 1024) {
                inode->block_ptr_2 = alloc_block();
                memset(blocks_2nd, 0, 1024 * 4);
            }

            if (c_2nd_c == 0) {
                blocks_2nd[c_2nd] = alloc_block();
                printf("new 2nd block: %d\n", blocks_2nd[c_2nd]);
            }

            blocks_1st[c_2nd_c] = alloc_block();
            printf("new 1st block: %d\n", blocks_1st[c_2nd_c]);
            c_2nd_c++;

            if (c_2nd_c == 1024 - 1) {
                c_2nd++;
                c_2nd_c = 0;
            }

            write_block(fd, (char *) &blocks_2nd, 1024 * 4, inode->block_ptr_2);
            write_block(fd, (char *) &blocks_1st, 1024 * 4, blocks_2nd[c_2nd]);
            write_block(fd, (char *) (buffer + block_wrote * 4096), 1024 * 4, blocks_1st[block_wrote]);
        }
    }

    return size;

#undef c_1st
#undef c_1st_c
}

/*
    read from inode. 
    STEPS: 
        1. how many blocks we need? 
        2. read from 1st block

    WARNING: 
        1. ABS(buffer.size / 4096) = buffer.size / 4096
*/
unsigned int cfs_read(int fd, struct inode_d *inode, char *buffer, unsigned int max_size) {
    unsigned int block_need = ALIGNUP_4096(max_size) / 4096;
    unsigned int block_read;
    
    unsigned int blocks_1st[1024];
    unsigned int blocks_2nd[1024];
    unsigned int blocks_3rd[1024];

#define c_1st block_read
#define c_1st_c block_read
    unsigned int c_2nd = 0;
    unsigned int c_2nd_c = 0;

    unsigned int c_3rd = 0;
    unsigned int c_3rd_c = 0;

    for (block_read = 0; block_read < block_need; block_read++) {
        if (block_read < 1024) {
            read_block(fd, (char *) &blocks_1st, 1024 * 4, inode->block_ptr_1);
            printf("read from %d\n", inode->block_ptr_1);

            read_block(fd, (char *) (buffer + block_read * 4096), 1024 * 4, blocks_1st[block_read]);
            printf("read from %d\n", blocks_1st[block_read]);
        }

        else if (block_read >= 1024 < 1048576) {
            c_2nd_c++;

            if (c_2nd_c == 1024 - 1) {
                c_2nd++;
                c_2nd_c = 0;
            }
            read_block(fd, (char *) &blocks_2nd, 1024 * 4, inode->block_ptr_2);
            read_block(fd, (char *) &blocks_1st, 1024 * 4, blocks_2nd[c_2nd_c]);

            read_block(fd, (char *) (buffer + block_read * 4096), 1024 * 4, blocks_1st[block_read]);
        }
    }

    return max_size;
}
