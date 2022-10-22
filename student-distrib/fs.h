#ifndef FS_H
#define FS_H

#include "lib.h"

#define DATABLOCK_SIZE 4096 //4kb data blocks
#define MAX_BYTES_IN_BLOCK (DATABLOCK_SIZE/4) //each entry is 4bytes
#define NUM_POSSIBLE_ENTRIES (DATABLOCK_SIZE/64)-1 //each directory entry is 64bytes
#define NUM_DATABLOCK_ENTRIES MAX_BYTES_IN_BLOCK-1 // First value in inode is length rest are datablocks

typedef struct dentry_object 
{
    char file_name[32];//32 Byte file size
    uint32_t file_type;
    uint32_t inode_number;
    char reserved[24];//24 Bytes reserved in dentry
} __attribute__ ((packed));

typedef struct boot_block 
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;       
    uint32_t num_datablocks;
    char reserved[52];//52 Bytes reserved in boot_block
    struct dentry_object dir_entries[NUM_POSSIBLE_ENTRIES];
} __attribute__ ((packed));

typedef struct inode 
{
    uint32_t length;
    uint32_t data_block_number[NUM_DATABLOCK_ENTRIES];
} __attribute__ ((packed));

typedef struct data_block 
{
    uint32_t data_blocks[MAX_BYTES_IN_BLOCK];
} __attribute__ ((packed));
 
#endif /* FS_H */
