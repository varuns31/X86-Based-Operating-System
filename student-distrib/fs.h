#ifndef FS_H
#define FS_H

#include "lib.h"

#define DATABLOCK_SIZE 4096 //4kb data blocks
#define ABN_JUMP DATABLOCK_SIZE/4
#define MAX_BYTES_IN_BLOCK (DATABLOCK_SIZE/4) //each entry is 4bytes
#define NUM_POSSIBLE_ENTRIES (DATABLOCK_SIZE/64)-1 //each directory entry is 64bytes
#define NUM_DATABLOCK_ENTRIES MAX_BYTES_IN_BLOCK-1 // First value in inode is length rest are datablocks

extern void create_boot_block();

extern void create_file_array();

typedef struct __attribute__ ((packed))
{
    uint8_t file_name[32];//32 Byte file size
    uint32_t file_type;
    uint32_t inode_number;
    char reserved[24];//24 Bytes reserved in dentry
}  dentry_t;

typedef struct __attribute__ ((packed))
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;       
    uint32_t num_datablocks;
    char reserved[52];//52 Bytes reserved in boot_block
    dentry_t dir_entries[NUM_POSSIBLE_ENTRIES];
}  boot_block;

typedef struct __attribute__ ((packed)) 
{
    uint32_t length;
    uint32_t data_block_number[NUM_DATABLOCK_ENTRIES];
} inode;

typedef struct __attribute__ ((packed)) 
{
    uint32_t data_blocks[MAX_BYTES_IN_BLOCK];
} data_block;

typedef struct __attribute__ ((packed))
{
    uint32_t* table_ptr;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
}  fd_node;


extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t * dentry);

extern int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry);

extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
 
extern int32_t fs_read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t fs_open (const uint8_t* filename);

extern int32_t fs_close (int32_t fd);

extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t directory_write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t directory_open (const uint8_t* filename);

extern int32_t directory_close (int32_t fd);


extern boot_block * our_boot_block;
extern uint32_t* abn_ptr;//absolute block number pointer
extern uint32_t* inode_ptr;
extern uint32_t* data_block_ptr;
extern unsigned int data_offset;

#endif /* FS_H */
