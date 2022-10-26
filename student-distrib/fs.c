#include "fs.h"
#include "lib.h"

// define global variables for the filsystem
boot_block * our_boot_block;
uint32_t* abn_ptr;//absolute block number pointer
uint32_t* inode_ptr;
uint32_t* data_block_ptr;
unsigned int data_offset = 1;

// create filesystem array node : used in Checkpoint 3.1
fd_node file_array[FILE_ARRAY_SIZE]; 
// create a temp_dentry to retrieve the dentry object created
dentry_t temp_dentry;

/* 
 * create_file_array
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
void create_file_array(){
    int i;
    // to handle stdin and stdout
    file_array[0].flags = 1;
    file_array[1].flags = 1;

    // set 2 to 8 of the filesystem flag as available
    for(i = 2; i < FILE_ARRAY_SIZE; i++){
        // initialize the file array to default values
        file_array[i].table_ptr = NULL;
        file_array[i].inode_num = 0;
        file_array[i].file_pos = 0;
        file_array[i].flags = 0; 
    }
}

/* 
 * read_dentry_by_name
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t * dentry)
{
   
    // get string length
    unsigned int flength = 0;

    // get the length of the file or the max size of the file = 32
    while(fname[flength] != '\0' && flength < MAX_FILE_NAME_LENGTH)
        flength++;

    // iterate through the boot block to find the dentry that corresponds to the file
    int i;
    for(i = 0; i <= NUM_POSSIBLE_ENTRIES; i++){
        uint8_t * cur_file = our_boot_block->dir_entries[i].file_name;
        // check if the filename matches
        if(!strncmp((int8_t*)cur_file,(int8_t*)fname, flength)){
            // if it matches, copy the dentry to the input argument and return
            memcpy((void*)dentry, (void*)(&(our_boot_block->dir_entries[i])), DENTRY_BLOCK_SIZE);
            return 0;
        }
    }
    // if the file is not found, return an error message
    return -1;
}

/* 
 * read_dentry_by_index
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry)
{
    // check if index is within bounds
    if(0 <= index && index <= NUM_POSSIBLE_ENTRIES){
        // check is there is a bad file
        if(our_boot_block->dir_entries[index].file_name[0] == '\0'){
            // return error message
            return -1;
        } 
        // copy the dentry at index to the input argument dentry and return
        memcpy((void*)dentry, (void*)(&(our_boot_block->dir_entries[index])), DENTRY_BLOCK_SIZE);
        return 0;
    }
    // else return error message
    return -1;
}

/* 
 * create_boot_block
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
void create_boot_block(fs_mod_start){
    // set the global variables defined to default values
    
    // set the boot block to the memory address of boot block
    our_boot_block = (boot_block *)fs_mod_start;
    // set the values of useful constants for read_data
    abn_ptr=(uint32_t*)fs_mod_start;
    data_block_ptr = (uint32_t*)fs_mod_start;
    data_block_ptr += ABN_JUMP * ((our_boot_block->num_inodes) + 1);
    inode_ptr = abn_ptr + ABN_JUMP;
}

/* 
 * read_data
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    // re-initialize useful constants to start values
    create_boot_block(abn_ptr);

    // cur_length is the return value which holds the number of bytes read
    uint32_t cur_length = 0;

    uint8_t* cur_buf_ptr = buf;

    // get pointer to the inode block of the file
    inode_ptr += ABN_JUMP*(inode);

    // get the current inode block and
    uint32_t* cur_block_num_ptr = inode_ptr + 1;
    cur_block_num_ptr += offset / DATABLOCK_SIZE;

    // get the length of the inode to check for the eof condition
    uint32_t inode_length = *(inode_ptr);
    inode_length -= offset;

    // handle for offset
    data_offset = offset % DATABLOCK_SIZE;
  
    // temp variable to get the block number
    uint32_t block_number;

    // check if we have reached length or end of file
    while(cur_length<=length || cur_length<=inode_length)
    {
        block_number = *(cur_block_num_ptr);
        uint8_t* cur_data_ptr =  (uint8_t*)(data_block_ptr + (ABN_JUMP * block_number));

        // start with offset
        if(cur_length == 0  && offset!= 0)
        {
            if(inode_length - (cur_length + data_offset) < DATABLOCK_SIZE && length - (cur_length + data_offset) >= DATABLOCK_SIZE)
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,inode_length - (cur_length + data_offset));
                return inode_length;
            }

            //Amount to read reached condition
            if(length - (cur_length + data_offset) < DATABLOCK_SIZE)
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,length - (cur_length + data_offset));
                return length;
            }
            // copy the block
            memcpy(cur_buf_ptr, cur_data_ptr + data_offset, DATABLOCK_SIZE - offset);
            // update number of bytes read
            cur_length += DATABLOCK_SIZE - data_offset;
            cur_buf_ptr += DATABLOCK_SIZE - data_offset;
            
        }

        //eof condition
        if(inode_length - cur_length < DATABLOCK_SIZE && length - cur_length >= DATABLOCK_SIZE)
        {
            // copy the block
            memcpy(cur_buf_ptr,cur_data_ptr,inode_length - cur_length);
            return inode_length;
        }

        //Amount to read reached condition
        if(length-cur_length < DATABLOCK_SIZE)
        {
            // copy the block
            memcpy(cur_buf_ptr,cur_data_ptr,length-cur_length);
            return length;
        }
        else
        {
            memcpy(cur_buf_ptr, cur_data_ptr, DATABLOCK_SIZE);
            // update number of bytes read
            cur_length += DATABLOCK_SIZE;
            cur_buf_ptr += DATABLOCK_SIZE;
        }

        // incriment the current block number
        cur_block_num_ptr++;
    }

    return cur_length;
}

/* 
 * fs_read
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t fs_read (int32_t fd, void* buf, int32_t nbytes){

    // if the buf is empty
    if(buf == NULL) return -1;

    if(file_array[fd].flags == 0){
        printf("Invalid fd\n");
        return -1;
    }

    // get the inode number from the filesystem struct
    unsigned int inode_num = file_array[fd].inode_num;
    // return the read_data 
    return read_data(inode_num, 0, buf, nbytes);
}

/* 
 * fs_write
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: error message
 *   SIDE EFFECTS: 
 */
int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes){
    printf("Read only file system\n");
    return -1;
}

/* 
 * fs_open
 *   DESCRIPTION: opens a file
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: fd
 *   SIDE EFFECTS: 
 */
int32_t fs_open (const uint8_t* filename){

    unsigned int flength = 0;
    int pos_idx = 0;
    char* cur_filename;
    int cur_inode;

    // get the length of the file or the max size of the file = 32
    // iterate through the boot block to find the dentry that corresponds to the file
    int i;
    for(i = 0; i <= NUM_POSSIBLE_ENTRIES; i++){
        uint8_t * cur_file = our_boot_block->dir_entries[i].file_name;
        // check if the filename matches
        if(!strncmp((int8_t*)cur_file,(int8_t*)filename, strlen(filename))){
            // if it matches, copy the dentry to the input argument and return
            cur_filename = our_boot_block->dir_entries[i].file_name;
            cur_inode=our_boot_block->dir_entries[i].inode_number;
            pos_idx=i;
            break;
        }
    }
    // check for error condition
    if(i > NUM_POSSIBLE_ENTRIES){
        printf("Invalid filename\n");
        return -1;
    }

    // for CP3.2, we will just use the first available fd which is 2
    int fd_idx;
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        if(file_array[fd_idx].inode_num == cur_inode && cur_inode != 0){
            puts(filename);
            printf(" file has already been opened\n");
            return -1;
        }
        if(file_array[fd_idx].flags == 0){
            // set the file array at specified location
            puts(filename);
            printf(" has been opened at position %d\n", fd_idx);
            file_array[fd_idx].inode_num = cur_inode;
            file_array[fd_idx].table_ptr = 0 ; // set to jmp table that we make later
            file_array[fd_idx].file_pos = pos_idx;
            file_array[fd_idx].flags = 1; // indicated in use
            return fd_idx;
        }
    }

    //    if(strncmp(filename, temp_dentry.file_name, strlen(filename)) == 0){
    //         puts(filename);
    //         printf(" file has already been opened\n");
    //         return -1;
    //     }


    printf("Error: Too many files are open\n");
    return -1;
}

/* 
 * fs_close
 *   DESCRIPTION: closes the file
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t fs_close (int32_t fd){
    // check if fd is within range of 2 to 7 becuase stdin and stdout are not allowed
    if(2 <= fd && fd < 8){
        file_array[fd].inode_num = 0;
        file_array[fd].table_ptr = 0 ; // set to jmp table that we make later
        file_array[fd].file_pos = 0;
        file_array[fd].flags = 0;
        return 0;
    }
    return -1;
}

/* 
 * directory_read
 *   DESCRIPTION: reads from a directory
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    // fill the buffer with the filename specified at index, to be modified in CP3.3
    memcpy(buf, our_boot_block->dir_entries[fd].file_name, 32);
    return 0;
}

/* 
 * directory_write
 *   DESCRIPTION: writes to a directory
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t directory_write (int32_t fd, const void* buf, int32_t nbytes){
    // to be implemented in CP3.3
    return 0;
}

/* 
 * directory_open
 *   DESCRIPTION:  opens a directory
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t directory_open (const uint8_t* filename){
    // to be implemented in CP3.3
    return 0;
}

/* 
 * directory_close
 *   DESCRIPTION: closes a directory
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t directory_close(int32_t fd){
    // check if fd in stdin or std out and return error message
    if(fd == 0 || fd == 1) return -1;
    // close file
    return 0;
}
