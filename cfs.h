#define ALIGNUP_4096(x) ((x + 0xfff) & 0xfffff000)

#define try(expr) if (expr < 0) { \
    printf("fatal error: call %s: %s\n", #expr, strerror(errno)); \
    exit(1); \
}



#define SUPERBLOCK_MAGIC 20090427
#define UNKNOW 0x0000


#define IMD_AP_ROOT_RD 0x0001
#define IMD_AP_ROOT_WR 0x0002
#define IMD_AP_ROOT_XR 0x0004

#define IMD_AP_USER_RD 0x0008
#define IMD_AP_USER_WR 0x0010
#define IMD_AP_USER_XR 0x0030

#define IMD_FT_REGFILE 0x0040
#define IMD_FT_FOLDER  0x0080
#define IMD_FT_BLKDEV  0x0100
#define IMD_FT_CHRDEV  0x0200
#define IMD_FT_POINTER 0x0400

/* NOTE! LBA address should be 24 or 48 bits, but we use 32bit! (because we use block) */

/* NOTE! It's in data block! */
struct inode_d {
    /* inode */
    unsigned int inode;
    /* inode mode */
    unsigned short mode;
    /* block count (if it's a pointer, point to inode)*/
    unsigned int block_count;
    /* offset to last block (block_count * 4096 + block_offset = file_size) */
    unsigned short block_size_offset;
    /* block pointer 1st */
    unsigned int block_ptr_1;
    /* block pointer 2nd */
    unsigned int block_ptr_2;
    /* block pointer 3rd */
    unsigned int block_ptr_3;
    /* reserved */
    char reserved[8];
};


/* NOTE! It's in data block too. */
struct dir_d {
    /* bitmap of inodes */
    unsigned char inode_bitmap[4096 / 8];
    /* inodes */
    struct inode_d inodes[4096];
    /* names */
    char names[4096 * 128];
    /* reserved (use 257 blocks) */
    char reserved[3584];
};

struct super_block_d {
    /* the first sector should be reserved anytime. */
    unsigned char mbr[512];
    /* 0x1328e3b */
    unsigned int magic;
    /* block count */
    unsigned int block_count;
    /* block bitmap */
    unsigned int block_bitmap;
    /* root dir block */
    unsigned int root_dir;
    /* oh, how can I use it? */
    unsigned char reserved[3568];
} superblock_d;

struct super_block_m {
    /* point to superblock in disk */
    struct super_block_d *sb_disk;
    /* point to block bitmap */
    unsigned char *block_bitmap;
    /* point to first data block */
    struct dir_d *root_dir;
} superblock_m;


struct cfs_m {
    /* a superblock */
    struct super_block_m *superblock;
    /* block bitmap */
    unsigned char *block_bitmap;
    /* and root */
    struct dir_d *root;
} cfs;

int cfs_format(int fd);
int cfs_init(int fd);
int cfs_writeback(int fd);
int cfs_ls(struct dir_d *dir);
struct inode_d *cfs_create(struct dir_d *dir, char *filename, unsigned short mode);
unsigned int alloc_block(void);
unsigned int cfs_write_block(int fd, struct inode_d *inode, unsigned int block_count, char *buffer, unsigned int size);
unsigned int cfs_read_block(int fd, struct inode_d *inode, unsigned int block_count, char *buffer, unsigned int size);
