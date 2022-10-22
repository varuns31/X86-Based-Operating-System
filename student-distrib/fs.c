#include "fs.h"



struct boot_block our_boot_block;
struct inode our_inodes[NUM_POSSIBLE_ENTRIES];
struct data_block our_data_blocks[NUM_DATABLOCK_ENTRIES*NUM_POSSIBLE_ENTRIES];
void init_file_driver()
{
    our_boot_block.num_dir_entries = 0;
    our_boot_block.num_inodes = 0;
    our_boot_block.num_datablocks = 0;
    
    our_boot_block.dir_entries[0].file_name[0] = '.';
    our_boot_block.dir_entries[0].file_type = 1;
    our_boot_block.dir_entries[0].inode_number = 0;

}