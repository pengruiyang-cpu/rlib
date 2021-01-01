#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rlib.h"


#define FILE_EXE_ROOT 0x01
#define FILE_RDB_ROOT 0x02
#define FILE_WDB_ROOT 0x04

#define FILE_EXE_USER 0x08
#define FILE_RDB_USER 0x10
#define FILE_WDB_USER 0x20

#define FILE_DIR 0x40

#define FILE_SPC 0x80
#define FILE_SPC_LNK 0x01


struct superblock {
    /* master boot record */
    char mbr[512];
    /* point to -> *start of root_dir (block) */
    unsigned int root_dir;
    /* how many blocks in this disk */
    unsigned int block_count;
    /* how can I use this?  */
    char reserved[3575];
};

struct inode {
	/* file or floder name */
	char name[128];
    /* file type */
    unsigned char file_type;
	/* blocks */
	unsigned int blocks[28];
	/* 1st block or special file */
	unsigned int block_1st;
	/* 2nd block */
	unsigned int block_2nd;
	/* 3rd block */
	unsigned int block_3rd;

    char reserved[3];
};

struct folder {
    /* start block of this folder */
    unsigned int block_start;
    /* inode bitmap 512B */
	char inode_bitmap[4096 / 8];
    /* inodes 1048576B */
	struct inode inodes[4096];
};

struct cfs {
    /* super block */
    struct superblock super_block;
    /* blocks bitmap */
	char *block_bitmap;
    /* root-dir 1049088B */
	struct folder *root_dir;
    /* data... */
    char *data;
};


/* 
    init / format disk fd

    STEPS: 
        1. format disk
            1. format super-block
                1. get disk size
                    if disk too small: failed
                2. get block count
                3. write it
                    1. write(read(mbr))
                    2. reserve root_dir
                    3. write(block count)
            
            2. format block-bitmap
                1. clean block-bitmap
                2. set block-bitmap.bit 0 (super-block)

            3. format root-dir
                1. clean root-dir
                2. create directory . and ..
                3. write
                4. set block-bitmap.bit 1 - 257
        2. init fs
            1. init super-block
                1. read it
                2. write it

            2. init block-bitmap
                1. read it
                2. write it
            
            3. init root-dir
                1. read it
                2. write it
*/
struct cfs *cfs_init(FILE *fd, char mode) {
    if (mode == 1) {
        unsigned int disk_size;
        unsigned int block_count;
        struct cfs *fs = (struct cfs *) malloc(sizeof(struct cfs));

        rewind(fd);
        fseek(fd, 0, SEEK_END);
        disk_size = ftell(fd);
        rewind(fd);

        block_count = disk_size / 4096;

#define CFS_HEADER_SIZE 4096 /* super-block */ + block_count / 8 /* block bitmap */ + sizeof(struct folder) /* root-dir */
        
        if (CFS_HEADER_SIZE >= disk_size) {
            printf("fatal error: input disk too small\n");
            return 0;
        }
#undef CFS_HEADER_SIZE

        fs->super_block.block_count = block_count;

        /* jump after MBR 512B */
        fseek(fd, 512, SEEK_CUR);
        fs->super_block.root_dir = (4096 + block_count / 8) / 4096;
        assert(fwrite(&fs->super_block.root_dir, 1, 4, fd) != 0);
        assert(fwrite(&block_count, 1, 4, fd) != 0);
        rewind(fd);


        /* jump after super-block */
        fseek(fd, 4096, SEEK_SET);

        char *block_bitmap = (char *) malloc(block_count / 8);
        assert(block_bitmap != NULL);
        memset(block_bitmap, 0, block_count / 8);

        bitmap_set(block_bitmap, 0, 1);

        assert(fwrite(block_bitmap, 4096, block_count / 8, fd) != 0);


        struct folder *root_dir = malloc(sizeof(struct folder));

        memset(root_dir, 0, sizeof(struct folder));

#define CFS_ROOT_DIR_OFFSET (4096 /* super-block */ + block_count / 8 /* block bitmap */) / 4096

        root_dir->block_start = CFS_ROOT_DIR_OFFSET;

#undef CFS_ROOT_DIR_OFFSET

        memset(root_dir->inode_bitmap, 0, 4096 / 8);
        memset(root_dir->inodes, 0, 4096 * sizeof(struct inode));

        /* . */
        char filename[128] = {"."};
        strcpy(root_dir->inodes[0].name, filename);
        
        root_dir->inodes[0].file_type |= FILE_SPC;
        root_dir->inodes[0].file_type |= FILE_SPC_LNK;

        /* NOTE! if a linker file.point-to = 0, point to root-dir */
        root_dir->inodes[0].block_1st = 0;

        bitmap_set(root_dir->inode_bitmap, 0, 1);

        /* .. */
        filename[0] = '.';
        filename[1] = '.';

        strcpy(root_dir->inodes[1].name, filename);
        root_dir->inodes[1].file_type |= FILE_SPC;
        root_dir->inodes[1].file_type |= FILE_SPC_LNK;

        /* NOTE! if a linker file.point-to = 0, point to root-dir */
        root_dir->inodes[1].block_1st = 0;

        bitmap_set(root_dir->inode_bitmap, 1, 1);

        fs->block_bitmap = block_bitmap;
        fs->root_dir = root_dir;


        assert(fwrite(&(root_dir->block_start), 1, 4, fd) != 0);
        assert(fwrite(root_dir->inode_bitmap, 1, 4096 / 8, fd) != 0);
        assert(fwrite(root_dir->inodes, sizeof(struct inode), 4096, fd) != 0);

        rewind(fd);

        return fs;
    }

    else {
        struct cfs *fs = (struct cfs *) malloc(sizeof(struct cfs));
        fs->root_dir = (struct folder *) malloc(sizeof(struct folder));

        rewind(fd);
        
        fread(fs->super_block.mbr, 1, 512, fd);
        fread(&(fs->super_block.root_dir), 1, 4, fd);
        fread(&(fs->super_block.block_count), 1, 4, fd);

        fs->block_bitmap = (char *) malloc(fs->super_block.block_count / 8);
        fread(fs->block_bitmap, 1, fs->super_block.block_count / 8, fd);
        fread(&(fs->root_dir->block_start), 1, 4, fd);
        fread(fs->root_dir->inode_bitmap, 1, 4096 / 8, fd);
        fread(fs->root_dir->inodes, sizeof(struct inode), 4096, fd);

        return fs;
    }
}


/*
    create a file to dir. 

    STEPS: 
        1. find a free inode
        2. write file name to inode
        3. write file type to inode
        4. write inode bitmap
*/
int cfs_create_file_mem(struct folder *dir, char *filename, unsigned char type) {
    int inode;

    for (inode = 0; inode < 4096 && bitmap_read(dir->inode_bitmap, inode) != 0; inode++);
    bitmap_set(dir->inode_bitmap, inode, 1);

    strcpy(dir->inodes[inode].name, filename);
    dir->inodes[inode].file_type = type;

    return inode;
}


/*
    create a file to fd

    STEPS:
        1. create a file in memory
        2. write to fd
            1. write inode-bitmap
            2. write inode changed
*/
int cfs_create_file(FILE *fd, struct folder *dir, char *filename, unsigned char type) {
    unsigned int inode;

    rewind(fd);

    inode = cfs_create_file_mem(dir, filename, type);

    fseek(fd, (dir->block_start) * 4096, SEEK_SET);

    assert(fwrite(&(dir->block_start), 1, 4, fd) != 0);
    assert(fwrite(dir->inode_bitmap, 1, 4096 / 8, fd) != 0);
    assert(fwrite(dir->inodes, sizeof(struct inode), 4096, fd) != 0);

    return inode;
}

int main(int argc, const char **argv) {
    FILE *fd = fopen(argv[2], "rb+");
    struct cfs *fs;
    assert(fd != NULL);

    if (strcmp(argv[1], "format") == 0) {
        fs = cfs_init(fd, 1);
    }

    else {
        fs = cfs_init(fd, 0);
    }

    int i;

    for (i = 0; i < 4096; i++) {
        //if (bitmap_read(fs->root_dir->inode_bitmap, i)) {
            printf("%d: %s\n", i, fs->root_dir->inodes[i].name);
        //}
    }

    fclose(fd);
    return 0;
}