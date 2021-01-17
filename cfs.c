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
    int fd = open(argv[1], O_RDWR);
    try(fd);


    if (strcmp(argv[2], "format") == 0) {
        cfs_format(fd);
        close(fd);
        return 0;
    }

    cfs_init(fd);
    char filename[128] = {"newfile.txt"};

    struct inode_d *inode = cfs_create(cfs.root, filename, 
                                IMD_AP_ROOT_RD | 
                                IMD_AP_ROOT_WR | 
                                IMD_AP_ROOT_XR | 
                                IMD_AP_USER_RD | 
                                IMD_AP_USER_WR | 
                                IMD_AP_USER_XR | 
                                IMD_FT_REGFILE
    );

    cfs_ls(cfs.root);

    char buffer_w[4096] = {"hello, world"};
    cfs_write(fd, inode, buffer_w, 0, 4096);

    cfs_writeback(fd);

    char buffer_r[4096] = {0};
    cfs_read_block(fd, inode, 1024, buffer_r, 4096);
    printf("%s\n", buffer_r);

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

    #define BLOCK_USED 1 /* superblock */ + ALIGNUP_4096(block_count / 8) / 4096 + sizeof(struct dir_d) / 4096

    int i;
    for (i = 1; i < BLOCK_USED; i++) {
        bitmap_set(cfs.block_bitmap, i - 1, 1);
    }

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

    char zero[1024] = {};
    char zero_filename[128 - 1] = {"zero.raw.chr"}
    
    struct inode_d *inode = cfs_create(root, zero_filename, 
                                IMD_AP_ROOT_RD | 
                                IMD_AP_ROOT_WR | 
                                IMD_AP_ROOT_XR | 
                                IMD_AP_USER_RD | 
                                IMD_AP_USER_WR | 
                                IMD_AP_USER_XR | 
                                IMD_FT_CHRDEV
    );
    cfs_write_block(fd, inode, 1, zero, 4096);
    
    superblock_m.block_bitmap = cfs.block_bitmap;
    superblock_m.root_dir = root;


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

    try(fsync(fd));

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
    printf("read from %d (%d) \n", block * 4096, block);
    lseek(fd, now_pos, SEEK_SET);
    return 0;
}

int write_block(int fd, void *buffer, unsigned int size, unsigned int block) {
    __off_t now_pos = lseek(fd, 0, SEEK_CUR);
    lseek(fd, block * 4096, SEEK_SET);
    try(write(fd, buffer, size));
    printf("write to %d (%d) \n", block * 4096, block);
    lseek(fd, now_pos, SEEK_SET);
    try(fsync(fd));
    return 0;
}

unsigned int cfs_write_block(int fd, struct inode_d *inode, unsigned int block_pos, char *buffer, unsigned int size) {
    if (block_pos < 1024) {
        unsigned int block_ptr_1[1024];
        unsigned int bp1_index;
        if (block_pos == 1) {
            inode->block_ptr_1 = alloc_block();
        }
        
        bp1_index = block_pos - 1;

        read_block(fd, block_ptr_1, 4096, inode->block_ptr_1);
        block_ptr_1[bp1_index] = alloc_block();
        write_block(fd, block_ptr_1, 4096, inode->block_ptr_1);
        read_block(fd, block_ptr_1, 4096, inode->block_ptr_1);

        write_block(fd, buffer, size, block_ptr_1[bp1_index]);
        return size;
    }
}

unsigned int cfs_read_block(int fd, struct inode_d *inode, unsigned int block_pos, char *buffer, unsigned int size) {
    if (block_pos < 1024) {
        unsigned int block_ptr_1[1024];
        unsigned int bp1_index;

        bp1_index = block_pos - 1;
        read_block(fd, block_ptr_1, 4096, inode->block_ptr_1);

        read_block(fd, block_ptr_1, 4096, inode->block_ptr_1);

        read_block(fd, buffer, size, block_ptr_1[bp1_index]);
        return size;
    }
}
