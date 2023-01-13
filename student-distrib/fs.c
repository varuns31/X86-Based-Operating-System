#include "fs.h"
#include "lib.h"
#include "pcb.h"
#include "keyboard.h"
#include "paging.h"

#define MAX_FILENAME_SIZE 32
#define NUM_COLS_SCREEN 24
#define NUM_ROWS_SCREEN 80
// define global variables for the filsystem
boot_block* our_boot_block;

// absolute block number pointer
uint32_t* abn_ptr;
uint32_t* inode_ptr;
uint32_t* data_block_ptr;

// absolute block number pointer (by docs)
unsigned int data_offset = 1;
extern int ATTRIB;

// create a temp_dentry to retrieve the dentry object created
dentry_t temp_dentry;

/* 
 * read_dentry_by_name
 *   DESCRIPTION: read dentry by file name
 *   INPUTS: file name and dentry
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: copies to dentry obj
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t * dentry)
{
   
    // get string length
    unsigned int flength = strlen((int8_t*) fname);
    if(flength >= MAX_FILENAME_SIZE) flength = MAX_FILENAME_SIZE;

    //LENGTH OF FILE IS 0
    if(flength == 0) return -1; 
    

    // iterate through the boot block to find the dentry that corresponds to the file
    int i;
    for(i = 0; i <= NUM_POSSIBLE_ENTRIES; i++) {

        uint8_t * cur_file = our_boot_block->dir_entries[i].file_name;
        unsigned int target_file_name = 0;
        target_file_name = strlen((int8_t*) cur_file);
        if ((target_file_name) >MAX_FILENAME_SIZE) target_file_name = MAX_FILENAME_SIZE;
        
        // check if the filename matches
        if((target_file_name == flength) && !strncmp((int8_t*)cur_file, (int8_t*)fname, target_file_name)){
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
 *   DESCRIPTION: get dentry by index
 *   INPUTS: dentry object
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t * dentry)
{
    // check if index is within bounds
    if(0 <= index && index <= NUM_POSSIBLE_ENTRIES){
        // check is there is a bad file
        if(our_boot_block->dir_entries[index].file_name[0] == '\0'){
            // return error message
            memcpy((void*)dentry, (void*)(&(our_boot_block->dir_entries[index])), DENTRY_BLOCK_SIZE);
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
 *   DESCRIPTION: creates and copies the boot block to local
 *   INPUTS: starting addr of filesystem
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: inits basic ptrs required
 */
void create_boot_block(fs_mod_start){
    // set the global variables defined to default values
    
    // set the boot block to the memory address of boot block
    our_boot_block = (boot_block *)fs_mod_start;
    // set the values of useful constants for read_data
    abn_ptr=(uint32_t*)fs_mod_start;
    data_block_ptr = (uint32_t*)fs_mod_start;

    //Data block pointer points to the block after the last inode
    data_block_ptr += ABN_JUMP * ((our_boot_block->num_inodes) + 1);
    inode_ptr = abn_ptr + ABN_JUMP;
}

/* 
 * read_data
 *   DESCRIPTION: reads data of a file when given the inode
 *   INPUTS: inode, offset, buff and length to read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 *   SIDE EFFECTS: writes to the buffer
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

    if(offset > inode_length)return 0;

    // handle for offset
    data_offset = offset % DATABLOCK_SIZE;
  
    // temp variable to get the block number
    uint32_t block_number;
    uint32_t read = 0;

    // check if we have reached length or end of file
    while(cur_length<=length || cur_length<=inode_length)
    {
        block_number = *(cur_block_num_ptr);
        uint8_t* cur_data_ptr =  (uint8_t*)(data_block_ptr + (ABN_JUMP * block_number));
        //printf_keyboard("Pointer Address %d \n" , (uint8_t) cur_data_ptr);

        // start with offset
        if(cur_length == 0  && offset!= 0)
        {
            //eof condition
            if(inode_length - (offset) <= DATABLOCK_SIZE && length >= inode_length - offset && ((DATABLOCK_SIZE-data_offset) >= inode_length - (offset)) )
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,inode_length - (offset));
                return inode_length - offset;
            }
            //Amount to read reached condition
            if((length + data_offset) <= DATABLOCK_SIZE)
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr + data_offset,length);
                return length;
            }
            // copy the block
            memcpy(cur_buf_ptr, cur_data_ptr + data_offset, DATABLOCK_SIZE - data_offset);
            // update number of bytes read
            cur_length += DATABLOCK_SIZE - data_offset;
            cur_buf_ptr += DATABLOCK_SIZE - data_offset; 
            read += DATABLOCK_SIZE - data_offset; 
            offset = 0;
        }
        else
        {
            //eof condition
            if(inode_length - cur_length <= DATABLOCK_SIZE && length > inode_length)
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr,inode_length - cur_length);
                return read + inode_length - cur_length;
            }

            //Amount to read reached condition
            if(length-cur_length < DATABLOCK_SIZE)
            {
                // copy the block
                memcpy(cur_buf_ptr,cur_data_ptr,length-cur_length);
                return read + length - cur_length;
            }
            else
            {
                memcpy(cur_buf_ptr, cur_data_ptr, DATABLOCK_SIZE);
                // update number of bytes read
                cur_length += DATABLOCK_SIZE;
                cur_buf_ptr += DATABLOCK_SIZE;
                read += DATABLOCK_SIZE;
            }
        }
        // incriment the current block number
        cur_block_num_ptr++;
    }

    return read;
}


/* 
 *   write_data
 *   DESCRIPTION: reads data of a file when given the inode
 *   INPUTS: inode, offset, buff and length to read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 *   SIDE EFFECTS: writes to the buffer
 */
int32_t write_data (uint32_t inode, uint8_t* buf, uint32_t length)
{
    // re-initialize useful constants to start values
    create_boot_block(abn_ptr);

    // cur_length is the return value which holds the number of bytes read
    uint32_t cur_length = 0;

    // get pointer to the inode block of the file
    inode_ptr += ABN_JUMP*(inode);

    // get the current inode block and
    uint32_t* cur_block_num_ptr = inode_ptr + 1;

    // get the length of the inode to check for the eof condition
    uint32_t inode_length = *(inode_ptr);
    *(inode_ptr) = 1200;
    uint8_t* cur_buf_ptr = buf;

    // temp variable to get the block number
    uint32_t block_number;
    uint32_t written = length;

    block_number = *(cur_block_num_ptr);
    uint8_t* cur_data_ptr =  (uint8_t*)(data_block_ptr + (ABN_JUMP * block_number));
    memmove(cur_data_ptr, cur_buf_ptr, length);
    return written;
}

/* 
 * fs_read
 *   DESCRIPTION: reads a file
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 *   SIDE EFFECTS: writes to buffer
 */
int32_t fs_read (int32_t fd, void* buf, int32_t nbytes){

    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    // if the buf is empty
    if(buf == NULL) return -1;

    // 7 is the max file index in file array. 0 is min
    if(pcb->process_file_array[fd].flags == 0 || fd > 7 || fd < 0){
        printf("Invalid fd\n");
        return -1;
    }

    // get the inode number from the filesystem struct
    unsigned int inode_num = pcb->process_file_array[fd].inode_num;
    // return the read_data 

    int bytes_read = read_data(inode_num, pcb->process_file_array[fd].file_pos, buf, nbytes);

    pcb->process_file_array[fd].file_pos += bytes_read;

    return bytes_read;
}

/* 
 * fs_write
 *   DESCRIPTION: writes to a file
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: none
 */
int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    memmove((void *)((Video_memory_loc + cur_terminal) << mask12),(void *) (Video_memory_loc << mask12), PG_Size);
    vimexit = 1;
    int prev_attrib = ATTRIB;
    ATTRIB=0X0D;
    clear();
    set_screen(0,0);
    int ret_terminal = cur_terminal;
    cur_terminal = 0;
    printf_keyboard("Entered Vim: Use ctrl + k(save) or j (not save) to exit\n");
    puts_keyboard(buf);
    set_screen(0,1);
    set_cursor(0,1);
    nbytes = strlen(buf);
    sti();
    while(vimexit != 0)
    {

    }
    cur_terminal = ret_terminal;
    char temp_vid_buff [NUM_COLS_SCREEN * (NUM_ROWS_SCREEN - 1)];
    int32_t i;
    for (i = 0; i < NUM_COLS_SCREEN * (NUM_ROWS_SCREEN - 1); i++) {
        temp_vid_buff[i] = *(uint8_t *)(video_mem_keyboard + (i << 1));
    }
    char temp_buff[nbytes];
    memmove(temp_buff, buf , nbytes);
    char buff[nbytes];
    int32_t vid_pos = NUM_ROWS_SCREEN;
    int row = 2;
    //clear();
    for (i = 0; i < nbytes; i++) {
        if(temp_buff[i] == '\n' && i!=0)
        {
            vid_pos = (row * (NUM_ROWS_SCREEN));
            buff[i]='\n';
            i++;
            row++;
            //putc_keyboard('\n');
        }
        //if(buff[i]=='\0')buff[i]='x';
        buff[i]=temp_vid_buff[vid_pos];
        //putc_keyboard(buff[i]);
        vid_pos++;
    }
    //puts_keyboard(temp_vid_buff);
    temp_vid_buff[1200] = '\0';
    if(!nochange)
    {
        write_data(pcb->process_file_array[fd].inode_num,(temp_vid_buff + 80),1200);
    }
    nochange = 0;
    ATTRIB = prev_attrib;
    memmove((void *) (Video_memory_loc << mask12),(void *)((Video_memory_loc + cur_terminal) << mask12), PG_Size);
    return 0;
}

/* 
 * fs_open
 *   DESCRIPTION: opens a file
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: changes fd array at allotted index
 */
int32_t fs_open (const uint8_t* filename){
    int pos_idx = 0;
    uint8_t* cur_filename;
    int cur_inode;

    // check for error condition
    dentry_t buf;

    // check if the filename has a corresponding dentry otherwise return -1
    int check = read_dentry_by_name(filename, &buf);
    if(check == -1)return -1;
    cur_filename = buf.file_name;
    cur_inode= buf.inode_number;

    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);

    int fd_idx;
    // since 0 and 1 are in use already as per docs
    // 7 is the max file index in file array. 0 is min
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        // File is already open
        if(cur_inode != 0 && pcb->process_file_array[fd_idx].inode_num == cur_inode){
            return -1;
        }
        if(pcb->process_file_array[fd_idx].flags == 0){
            // set the file array at specified location
            pcb->process_file_array[fd_idx].inode_num = cur_inode;
            pcb->process_file_array[fd_idx].file_pos = pos_idx;
            pcb->process_file_array[fd_idx].flags = 1; // indicated in use
            return fd_idx;
        }
    }
    return -1;
}

/* 
 * fs_close
 *   DESCRIPTION: closes the file
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: changes fd array for the pcb
 */
int32_t fs_close (int32_t fd){
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    
    // check if fd is within range of 2 to 7 becuase stdin and stdout are not allowed
    if(2 <= fd && fd < 8 && pcb->process_file_array[fd].flags == 1){
        pcb->process_file_array[fd].inode_num = 0;
        pcb->process_file_array[fd].table_ptr = 0 ; 
        pcb->process_file_array[fd].file_pos = 0;
        pcb->process_file_array[fd].flags = 0;

        return 0;
    }
    return -1;
}

/* 
 * directory_read
 *   DESCRIPTION: reads from a directory
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: copies data into buffer
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    
    // 7 is the max file index in file array. 0 is min
    if(pcb->process_file_array[fd].flags == 0 || fd > 7 || fd < 0) {
        printf("Invalid fd\n");
        return -1;
    }

    int num_of_files_read = pcb->process_file_array[fd].file_pos;
    dentry_t dentry;

    int ret = read_dentry_by_index(num_of_files_read, &dentry);

    // invalid dentry
    if (ret == -1) return 0;

    // 32 is the max length of file name as per docs
    strncpy((int8_t*)buf, (int8_t*) dentry.file_name, MAX_FILENAME_SIZE);

    pcb->process_file_array[fd].file_pos += 1;

    int bytes_read = strlen((int8_t*) dentry.file_name);

    if (bytes_read > MAX_FILENAME_SIZE) {
        return MAX_FILENAME_SIZE;
    }

    return bytes_read;
}

/* 
 * directory_write
 *   DESCRIPTION: writes to a directory
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: none
 */
int32_t directory_write (int32_t fd, const void* buf, int32_t nbytes){
    // can't write on read only fs
    return -1;
}

/* 
 * directory_open
 *   DESCRIPTION:  opens a directory
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: uses fd array
 */
int32_t directory_open (const uint8_t* filename){

    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);

    int fd_idx;
    // since 0 and 1 are in use as per docs
    // 7 is the max file index in file array. 0 is min
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        if(pcb->process_file_array[fd_idx].flags == 0){
            // set the file array at specified location
            pcb->process_file_array[fd_idx].inode_num = 0;
            pcb->process_file_array[fd_idx].file_pos = 0;
            pcb->process_file_array[fd_idx].flags = 1; 
            // indicated in use
            return fd_idx;
        }
    }
    return -1;
}

/* 
 * directory_close
 *   DESCRIPTION: closes a directory
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on error 0 on success
 *   SIDE EFFECTS: none
 */
int32_t directory_close(int32_t fd){
    // close file
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    // check if fd is within range of 2 to 7 becuase stdin and stdout are not allowed
    if(2 <= fd && fd < 8 && pcb->process_file_array[fd].flags == 1){
        pcb->process_file_array[fd].inode_num = 0;
        pcb->process_file_array[fd].table_ptr = 0 ; // set to jmp table that we make later
        pcb->process_file_array[fd].file_pos = 0;
        pcb->process_file_array[fd].flags = 0;
        return 0;
    }
    return -1;
}
