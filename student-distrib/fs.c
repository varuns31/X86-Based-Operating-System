#include "fs.h"
#include "lib.h"

boot_block * our_boot_block;
uint32_t* abn_ptr;//absolute block number pointer
uint32_t* inode_ptr;
uint32_t* data_block_ptr;
unsigned int data_offset = 1;


// create filesystem array node
fd_node file_array[8];
dentry_t temp_dentry;

void create_file_array(){
    int i;
    // to handle stdin and stdout
    file_array[0].flags = 1;
    file_array[1].flags = 1;

    for(i = 2; i < 8; i++){
        file_array[i].table_ptr = NULL;
        file_array[i].inode_num = 0;
        file_array[i].file_pos = 0;
        file_array[i].flags = 0;
    }
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


void create_boot_block(fs_mod_start){

   our_boot_block = (boot_block *)fs_mod_start;
   abn_ptr=(uint32_t*)fs_mod_start;
   data_block_ptr = (uint32_t*)fs_mod_start;
   data_block_ptr += ABN_JUMP * ((our_boot_block->num_inodes) + 1);
   inode_ptr = abn_ptr + ABN_JUMP;
}

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    create_boot_block(abn_ptr);

    uint32_t cur_length = 0;

    uint8_t* cur_buf_ptr = buf;

    inode_ptr += ABN_JUMP*(inode);

    uint32_t* cur_block_num_ptr = inode_ptr + 1;
    cur_block_num_ptr += offset / 4096;

    uint32_t inode_length = *(inode_ptr);
    inode_length -= offset;

    data_offset = offset % 4096;
  
    uint32_t block_number;

    while(cur_length<=length || cur_length<=inode_length)
    {
        block_number = *(cur_block_num_ptr);
        uint8_t* cur_data_ptr =  (uint8_t*)(data_block_ptr + (ABN_JUMP * block_number));

        // start with offset
        if(cur_length == 0  && offset!= 0)
        {
            if(inode_length - (cur_length + data_offset) < 4096 && length - (cur_length + data_offset) >= 4096)
            {
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,inode_length - (cur_length + data_offset));
                return inode_length;
            }

            //Amount to read reached condition
            if(length - (cur_length + data_offset) < 4096)
            {
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,length - (cur_length + data_offset));
                return length;
            }
            memcpy(cur_buf_ptr, cur_data_ptr + data_offset, 4096 - offset);
            cur_length += 4096 - data_offset;
            cur_buf_ptr += 4096 - data_offset;
            
        }

        //eof condition
        if(inode_length - cur_length < 4096 && length - cur_length >= 4096)
        {
            memcpy(cur_buf_ptr,cur_data_ptr,inode_length - cur_length);
            return inode_length;
        }

        //Amount to read reached condition
        if(length-cur_length < 4096)
        {
            memcpy(cur_buf_ptr,cur_data_ptr,length-cur_length);
            return length;
        }
        else
        {
            memcpy(cur_buf_ptr, cur_data_ptr, 4096);
            cur_length += 4096;
            cur_buf_ptr += 4096;
        }
        cur_block_num_ptr++;
    }

    return cur_length;
}


int32_t fs_read (int32_t fd, void* buf, int32_t nbytes){

    if(file_array[fd].flags == 0){
        printf("Invalid fd\n");
        return -1;
    }

    unsigned int inode_num = file_array[fd].inode_num;
    read_data(inode_num, 0, buf, nbytes);

    return 0;
}

int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

int32_t fs_open (const uint8_t* filename){
    int32_t temp_val = read_dentry_by_name(filename, &temp_dentry);

    if(temp_val != 0){
        printf("Invalid filename\n");
        return -1;
    }


    int fd_idx = 2;

    // set the file array at specified location
    file_array[fd_idx].inode_num = temp_dentry.inode_number;
    file_array[fd_idx].table_ptr = 0 ; // set to jmp table that we make later
    file_array[fd_idx].file_pos = fd_idx;
    file_array[fd_idx].flags = 1; // indicated in use

    return fd_idx;
}

int32_t fs_close (int32_t fd){
    if(2 <= fd && fd < 8){
        file_array[fd].inode_num = 0;
        file_array[fd].table_ptr = 0 ; // set to jmp table that we make later
        file_array[fd].file_pos = 0;
        file_array[fd].flags = 0;
        return 0;
    }
    return -1;
}

int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    int i = 0;
    for(i = 0; i < 15; i++){
        printf("File Name: ");
        int j = 32 - strlen(our_boot_block->dir_entries[i].file_name);
        while(j){
            putc(' ');
            j--;
        }

        // print name
        puts(our_boot_block->dir_entries[i].file_name);
        printf(", file type: %d", our_boot_block->dir_entries[i].file_type);

        // print size
        // create_boot_block(abn_ptr);
        inode_ptr = abn_ptr + ABN_JUMP;
        inode_ptr += ABN_JUMP*(our_boot_block->dir_entries[i].inode_number);
        uint32_t inode_length = *(inode_ptr);
        printf(", file size: %d", inode_length);
        
        putc('\n');
    }

    
}
