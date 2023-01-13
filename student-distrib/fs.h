#ifndef FS_H
#define FS_H

#include "lib.h"

#define DATABLOCK_SIZE 4096                         //4kb data blocks
#define ABN_JUMP DATABLOCK_SIZE/4
#define MAX_BYTES_IN_BLOCK (DATABLOCK_SIZE/4)       //each entry is 4bytes
#define NUM_POSSIBLE_ENTRIES (DATABLOCK_SIZE/64)-1  //each directory entry is 64bytes
#define NUM_DATABLOCK_ENTRIES MAX_BYTES_IN_BLOCK-1  // First value in inode is length rest are datablocks
#define FILE_ARRAY_SIZE 8
#define MAX_FILE_NAME_LENGTH 32
#define DENTRY_BLOCK_SIZE 64

extern void create_boot_block();

typedef struct __attribute__ ((packed)){
    uint8_t file_name[32];                           //32 Byte file size
    uint32_t file_type;
    uint32_t inode_number;
    char reserved[24];                                //24 Bytes reserved in dentry
}  dentry_t;

typedef struct __attribute__ ((packed)){
    uint32_t num_dir_entries;
    uint32_t num_inodes;       
    uint32_t num_datablocks;
    char reserved[52];                                  //52 Bytes reserved in boot_block
    dentry_t dir_entries[NUM_POSSIBLE_ENTRIES];
}  boot_block;

typedef struct __attribute__ ((packed)) {
    uint32_t length;
    uint32_t data_block_number[NUM_DATABLOCK_ENTRIES];
} inode;

typedef struct __attribute__ ((packed)) {
    uint32_t data_blocks[MAX_BYTES_IN_BLOCK];
} data_block;

typedef struct __attribute__ ((packed)){
    uint32_t* table_ptr;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
}  fd_node;

extern fd_node file_array[FILE_ARRAY_SIZE]; 

/* gets dentry by name */
extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t * dentry);

/* gets dentry by index */
extern int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry);

/* reads data blocks */
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* reads a file */
extern int32_t fs_read (int32_t fd, void* buf, int32_t nbytes);

/* writes to file */
extern int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes);

/* opens file */
extern int32_t fs_open (const uint8_t* filename);

/* closes file */
extern int32_t fs_close (int32_t fd);

/* reads directory */
extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

/* writes directory */
extern int32_t directory_write (int32_t fd, const void* buf, int32_t nbytes);

/* opens directory */
extern int32_t directory_open (const uint8_t* filename);

/* closes directory */
extern int32_t directory_close (int32_t fd);

extern boot_block * our_boot_block;
extern uint32_t* abn_ptr;
extern uint32_t* inode_ptr;
extern uint32_t* data_block_ptr;
extern unsigned int data_offset;
extern volatile int vimexit;
extern volatile int nochange;

#endif /* FS_H */
