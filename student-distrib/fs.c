#include "fs.h"
#include "lib.h"

boot_block * our_boot_block;
uint32_t* abn_ptr;//absolute block number pointer
uint32_t* data_block_ptr;

// inode our_inodes[NUM_POSSIBLE_ENTRIES];
// data_block our_data_blocks[NUM_DATABLOCK_ENTRIES*NUM_POSSIBLE_ENTRIES];


// void init_file_driver()
// {
//     our_boot_block.num_dir_entries = 0;
//     our_boot_block.num_inodes = 0;
//     our_boot_block.num_datablocks = 0;
    
//     our_boot_block.dir_entries[0].file_name[0] = '.';
//     our_boot_block.dir_entries[0].file_type = 1;
//     our_boot_block.dir_entries[0].inode_number = 0;

// }

void create_boot_block(fs_mod_start){
   our_boot_block = (boot_block *)fs_mod_start;
   abn_ptr=(uint32_t*)fs_mod_start;
   data_block_ptr = (uint32_t*)fs_mod_start;
   data_block_ptr += ABN_JUMP * ((our_boot_block->num_inodes) + 1);
}

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t * dentry)
{
   
    // get string length
    unsigned int flength = 0;

    while(fname[flength] != '\0')
        flength++;

    int i;
    for(i = 0; i <= NUM_POSSIBLE_ENTRIES; i++){
        uint8_t * cur_file = our_boot_block->dir_entries[i].file_name;
        if(!strncmp((int8_t*)cur_file,(int8_t*)fname, flength)){
            memcpy((void*)dentry, (void*)(&(our_boot_block->dir_entries[i])), 512);
            return 0;
        }
    }
    return -1;
}

int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry)
{
    if(0 <= index && index <= NUM_POSSIBLE_ENTRIES){
        if(our_boot_block->dir_entries[index].file_name[0] == '\0'){
            return -1;
        } 
        memcpy((void*)dentry, (void*)(&(our_boot_block->dir_entries[index])), 512);
        return 0;
    }

    return -1;
}


int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    abn_ptr+= ABN_JUMP*(inode+1);

    return 0;
}
