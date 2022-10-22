#ifndef FS_H
#define FS_H

#include "lib.h"

typedef struct dentry_object 
{
    char file_name[32];
    uint32_t file_type;
    uint32_t inode_number;
    char reserved[24];
} __attribute__ ((packed));

typedef struct boot_block 
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;       
    uint32_t num_datablocks;
    char reserved[52];
    struct dentry_object dir_entries[63];
} __attribute__ ((packed));

typedef struct inode 
{
    uint32_t length;
    uint32_t data_block_number[1023];
} __attribute__ ((packed));
 
#endif /* FS_H */
