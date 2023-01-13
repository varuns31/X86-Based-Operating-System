#include "syscall.h"

#define INT_MAX 2147483647              /* Maximum INT size in C (32 bit) */
#define MAX_FILE_NAME_LENGTH 32         /* Given as per docs */
#define ONE_KB 1024                     /* 1024 bytes */
#define ONE_MB 1024*1024                /* 1024 * 1024 bytes */
#define user_process_image 0x08048000   /* User program starts at 128 MB + offset*/
#define user_virtual_mem (128 * ONE_MB)
#define user_video_mem ((132 * ONE_MB) + (0xB8000))/* User video starts at 132 MB + video mem location*/           

extern volatile int piano;

// program eip stores the start of the program
uint8_t prog_eip[4];

// terminal pcbs
pcb_t term_pcb[3];

// file operations structure of read, write, open, and close
typedef struct file_ops_t {
    uint32_t (*system_read) (uint32_t fd, void* buf, uint32_t nbytes);
    uint32_t (*system_write) (uint32_t fd, const void* buf, uint32_t nbytes);
    uint32_t (*system_open) (const uint8_t* filename);
    uint32_t (*system_close) (uint32_t fd);
} file_ops_t;

// array of different read, write, open, and close functions for each file
file_ops_t file_ops_table[7] = {
    {(void *) terminal_read, (void *) terminal_write, (void *) terminal_open, (void *) terminal_close},
    {(void *) keyboard_read, (void *) keyboard_write, (void *) keyboard_open, (void *) keyboard_close}, 
    {(void *) rtc_read, (void *) rtc_write, (void *) rtc_open, (void *) rtc_close}, 
    {(void *) fs_read, (void *) fs_write, (void *) fs_open, (void *) fs_close}, 
    {(void *) directory_read, (void *) directory_write, (void *) directory_open, (void *) directory_close}, 
    {(void *) terminal_read, (void *) ret_fail, (void *) ret_fail, (void *) ret_fail}, 
    {(void *) ret_fail, (void *) terminal_write, (void *) ret_fail, (void *) ret_fail}, 
};

/* 
 * create_file_array
 *   DESCRIPTION: initiializes the given file array with stdin and stdout
 *   INPUTS: file_array for the pcb
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: inits fd array
 */
void create_file_array(fd_node *file_array){
    int i;

    // to handle stdin
    file_array[0].table_ptr = (void *) &file_ops_table[5];
    file_array[0].inode_num = 0;
    file_array[0].file_pos = 0;
    file_array[0].flags = 1;

    // to handle stdout
    file_array[1].table_ptr = (void *) &file_ops_table[6];
    file_array[1].inode_num = 0;
    file_array[1].file_pos = 0;
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
 * system_getargs
 *   DESCRIPTION: get the arguments from program and store in pcb
 *   INPUTS: buf -> store the arguments here. nbytes --> number of bytes 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t system_getargs (uint8_t* buf, int32_t nbytes) {
    //Invalid Pid
    if(curr_pid < 0) return 0;
  	
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);

    //NULL CHECKING && argument should be less than nbytes
    if(buf == NULL || strlen(pcb->pcb_arg) > nbytes || strlen(pcb->pcb_arg) == 0) return -1;

    //Put argument in buffer
    strcpy((char*) buf, (char*) pcb->pcb_arg);

    return 0;
}

/* 
 * system_vidmap
 *   DESCRIPTION: passes the user video memory into scree_start
 *   INPUTS: screen_start -> store the user video memory pointer here
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t system_vidmap (uint8_t** screen_start) {
    cli();
    //NULL Checking
    if(screen_start == NULL) return -1;

    //Invalid Pointer
    if((uint32_t)screen_start < (uint32_t)user_virtual_mem || (uint32_t)screen_start >= (uint32_t)user_virtual_mem + 4 * ONE_MB) return -1;

    video_paging_from_addr();

    //Map pointer to new user video location
    *screen_start = (uint8_t *)(user_video_mem);
    sti();
    return 0;
}

/* 
 * system_set_handler
 *   DESCRIPTION: not implemented
 *   INPUTS: signum and handler_address
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t system_set_handler (int32_t signum, void* handler_address) {
    return -1;
}

/* 
 * create_file_array
 *   DESCRIPTION: not implemented
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
int32_t system_sigreturn (void) {
    return -1;
}

/* 
 * system_halt
 *   DESCRIPTION: halts the currently running program
 *   INPUTS: status -> programmer defined value. 0 -> program halted correctly, 69 --> exception, -1 -> program not halted correctly
 *   OUTPUTS: none
 *   RETURN VALUE: 0 -> program halted correctly, 256 if there was an exception, -1 -> the program did not halt correctly. 
 *   SIDE EFFECTS: none
 */
int32_t system_halt (uint8_t status) {   
    cli();
    // printf("Halting curr pid %d\n\n\n",curr_pid);
    int32_t status_ret_val = 0;

    // status -> 0 to 255 then return 256 --> exception occured. Note that 69 means that the user program had an exception  
    if (status == 69) status_ret_val = 256;
    // status -> 0 then return 0 --> excecution completed
    if (status == 0) status_ret_val = 0;
    
    // find the pcb from curr_pid
    pcb_t* halt_pcb = (pcb_t *) find_pcb(curr_pid);
    
    // return -1 if the pcb does not exist
    if(halt_pcb == NULL) {
        printf("Error in halt - PCB not found!\n");
        return -1;
    }

    // clear the process file array for each pcb
    int i;
    for(i = 2; i < FILE_ARRAY_SIZE; i++) {
        // initialize the file array to default values
        if(halt_pcb -> process_file_array[i].flags == 1)
        {
            //Closing all open files in file_array if they weren't explicitly closed by user
            system_close(i);
        }
    }

    // teardown the argument buffer
    for(i = 0; i < 128; i++) {
        halt_pcb->pcb_arg[i] = 0;
    }

    // set pcb to inactive
    halt_pcb->active = 0;

    // check if the program that is requested to exit is the first shell, if it is then restart shell.
    if(halt_pcb->parent_id == -1) {
        //pit_curr_terminal = cur_terminal;
        system_execute((uint8_t*) "shell");
    }

    terminals[pit_curr_terminal].latest_pid = halt_pcb->parent_id;

    // set tss for parent
    tss.ss0 = KERNEL_DS;
    // calculating the addr w.r.t. 8 MB (given in the docs)
    tss.esp0 = (8 * ONE_MB) - (8 * ONE_KB * halt_pcb->parent_id) - sizeof(int);

    // clean child's page
    clean_page();

    // reset paging to parent's
    process_paging(halt_pcb->parent_id);

    // set current pid to parent pid
    curr_pid = halt_pcb->parent_id;
    terminals[pit_curr_terminal].latest_pid = curr_pid;

    // go back to execute before return
	asm volatile("mov %0, %%esp \n\
                mov %1, %%ebp \n\
                mov %2, %%eax \n\
                LEAVE \n\
                RET \n\
               "
               :
               : "r"(halt_pcb->base_esp), "r" (halt_pcb->base_ebp), "r"(status_ret_val)
               : "%eax"
               );

    return status_ret_val;
}

/* 
 * system_execute
 *   DESCRIPTION: executes the program from the commad
 *   INPUTS: command -> a string that we want to execute
 *   OUTPUTS: none
 *   RETURN VALUE: 0 -> executed properly, -1 --> execution failed, other values coming from halt
 *   SIDE EFFECTS: none
 */
int32_t system_execute (const uint8_t* command) {    
    cli();
    change_uvid_putc (cur_terminal, pit_curr_terminal);

    // local variables for context switching
    register int prev_esp asm("esp");
    register int prev_ebp asm("ebp");

    if(strncmp((char*)command, "piano", 5) == 0){
        memmove((void *)((Video_memory_loc + cur_terminal) << mask12),(void *) (Video_memory_loc << mask12), PG_Size);
        int prev_attrib = ATTRIB;
        ATTRIB=0X0D;
        clear();
        set_screen(0,0);
        int ret_terminal = cur_terminal;
        cur_terminal = 0;
        piano = 1;
        sti();
        while(piano != 0) {}
        cli();
        ATTRIB = prev_attrib;
        cur_terminal = ret_terminal;
        memmove((void *) (Video_memory_loc << mask12),(void *)((Video_memory_loc + cur_terminal) << mask12), PG_Size);
        return 0;
    }

    if(strncmp((char*)command, "pid", 3) == 0){
  	    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
        printf("curr pid: %d\n", pcb->process_id);
        printf("parent pid: %d\n", pcb->parent_id);
        return 0;
    }

    if(strncmp((char*)command, "switch", 6) == 0)
    {
        if(terminals[cur_terminal].og == 1)
        {
  	        changecolor(terminals[cur_terminal].attribute);
            terminals[cur_terminal].og = 0 ; 
        }
        else
        {
  	        changecolor(0x0F);
            terminals[cur_terminal].og = 1;
        }
        return 0;
    }

    int parent_id = curr_pid;
    
    int8_t command_deep_copy[128];
    strcpy((char *) command_deep_copy, (char *) command);

    /* Parse args - not needed */
    int size = strlen((char*) command);

    // 128 is the max buffer size
    char new_command[128];
    
    int cmd_idx;
    int count = 0;
    int first_char_found = 0;
    
    for(cmd_idx = 0; cmd_idx < size; cmd_idx++){
        // 32 is space
        if (command[cmd_idx] == 32 && first_char_found == 0) continue;
        if (command[cmd_idx] == 32 && first_char_found) break; 
        first_char_found = 1;
        new_command[count] = command[cmd_idx];
        count++;
    }
    
    new_command[count] = '\0';

    /* Check for executable */
    dentry_t exec_temp_dentry;
    int is_valid = -1;
    is_valid = read_dentry_by_name((uint8_t*) new_command, (dentry_t*) &exec_temp_dentry);
    if(is_valid == -1){
        // printf("Dentry got fed");
        return -1;
    }

    // read 4 bytes from program
    uint8_t exec_read_buf[4];
    read_data(exec_temp_dentry.inode_number, 1, exec_read_buf, 4);
    // 3 bytes to compare
    if(strncmp((int8_t*) "ELF", (int8_t*) exec_read_buf, 3) != 0){
        return -1;
    }

    // get the program start from the bytes 24 - 27 of the program and then set it to program eip.
    read_data(exec_temp_dentry.inode_number, 24, prog_eip, 27);

    /* Set up paging */
    curr_pid = get_first_inactive_pid();

    if(curr_pid == -1) {
        curr_pid = terminals[pit_curr_terminal].latest_pid;
        printf("Max number of processes reached, sawry :P\n");
        return 0;
    }

    if (curr_pid < 3) {
        parent_id = -1;
    }

    terminals[pit_curr_terminal].latest_pid = curr_pid;

    if (parent_id == -1) printf("Current Terminal: %d\n", pit_curr_terminal);

    process_paging(curr_pid);

    /* Load file into memory */
    uint8_t* exec_prog_img_addr = (uint8_t *) user_process_image;
    read_data(exec_temp_dentry.inode_number, 0, exec_prog_img_addr, INT_MAX);
    
    /* Create PCB */
    pcb_t curr_pcb;
    curr_pcb.process_id = curr_pid;
    curr_pcb.parent_id = parent_id;

    // note: int* changed to int
    curr_pcb.base_ebp = (int) prev_ebp;
    curr_pcb.base_esp = (int) prev_esp;
    curr_pcb.active = 1;

    count = 0;
    first_char_found = 0;
    int j;

    for(j = cmd_idx; j < size; j ++){
        // 32 is space
        if (command_deep_copy[j] == 32 && first_char_found == 0) continue;
        if (command_deep_copy[j] == 32 && first_char_found) break; 
        first_char_found = 1;
        curr_pcb.pcb_arg[count] = command_deep_copy[j];
        count++;
    }
    
    curr_pcb.pcb_arg[count] = '\0';  

    create_file_array(curr_pcb.process_file_array) ;
    set_pcb_by_pid(&curr_pcb, curr_pid);

    // set the tss esp and ss. Note that esp0 points to the next memory address
    tss.ss0 = KERNEL_DS;
    // calculating the addr w.r.t. 8 MB (given in the docs)
    tss.esp0 = (8 * ONE_MB) - (8 * ONE_KB * curr_pid) - sizeof(int);

    terminals[cur_terminal].ss0 = KERNEL_DS;
    terminals[cur_terminal].esp0 = (8 * ONE_MB) - (8 * ONE_KB * curr_pid) - sizeof(int);

    // Push IRET context to kernel stack
    jump_usermode();

    return 0;
}

/* 
 * system_read
 *   DESCRIPTION: calls the relevant read function
 *   INPUTS: fd - file descriptor for rtc
 *           buf - read data is put here
 *           nbytes - unused, but number of bytes read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fail, otherwise returns the number of bytes read
 *   SIDE EFFECTS: none
 */
int32_t system_read (int32_t fd, void* buf, int32_t nbytes) {   
  	pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);

    // invalid fd. max fd is 7, min fd is 0
    if(pcb->process_file_array[fd].flags == 0 || fd > 7 || fd < 0){
        return -1;
    }

    file_ops_t sys_func_table = *((file_ops_t*)(pcb->process_file_array[fd].table_ptr));

    int ret_val = sys_func_table.system_read(fd, buf, nbytes);

    return ret_val;
}

/* 
 * system_write
 *   DESCRIPTION: calls the relevant write function
 *   INPUTS: fd - file descriptor for rtc
 *           buf - read data is put here
 *           nbytes - unused, but number of bytes read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fail, otherwise returns the number of bytes written
 *   SIDE EFFECTS: none
 */
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes) {  
    cli();
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);

    // invalid fd, max fd is 7, min fd is 0
    if(pcb->process_file_array[fd].flags == 0 || fd > 7 || fd < 0){
        return -1;
    }

    file_ops_t sys_func_table = *((file_ops_t*)(pcb->process_file_array[fd].table_ptr));

    //Change OS name for Fun
    if(!strncmp("391OS>",buf,6))
    {
        printf("MastOS> ");
        return 8;
    }
     if(!strncmp("Starting 391 Shell",buf,18))
    {
        printf("Experiencing Mast OS:\n");
        return 20;
    }

    return sys_func_table.system_write(fd, buf, nbytes);
}

/* 
 * system open
 *   DESCRIPTION: initializes the table pointer in accordance to the file type
 *   INPUTS: filename - the name of the file
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fail, otherwise 0 for success
 *   SIDE EFFECTS: none
 */
int32_t system_open (const uint8_t* filename) {
    //if the filename is empty return fail

    if(strlen((int8_t*) filename) == 0) return -1;

    //get the current pcb
  	pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    
    // get the relevant dentry object to get what kind of file type it is
    uint32_t fd;
    dentry_t dentry;
    int ret = read_dentry_by_name(filename, &dentry);
    // printf("FIENAME: %s\n",dentry.file_name);

    // if the filename matches, set up the table pointer for relevant file
    if(!strncmp((int8_t*)filename, "terminal", strlen((char *) filename))){
        fd = terminal_open(filename);
        
        // sanity check for fd and setting up pcb's file pointer. normal fd is between 1 to 7
        if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[0];
        else return -1;
    } 
    else if (!strncmp((int8_t*)filename, "keyboard", strlen((char *) filename))) {
        fd = keyboard_open(filename);

        // sanity check for fd and setting up pcb's file pointer.normal fd is between 1 to 7
        if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[1];
        else return -1;
    } 
    else if (!strncmp((int8_t*)filename, "rtc", strlen((char *) filename))) {
        fd = rtc_open(filename);

        // sanity check for fd and setting up pcb's file pointer. normal fd is between 1 to 7
        if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[2];
        else return -1;
    } 
    else {
        // if the directory read by name returns -1, return -1
        if (ret == -1) return ret;

        // if the dentry is rtc then set up the file pointer to rtc
        if (dentry.file_type == 0) {
            fd = rtc_open(filename);
            
            // sanity check for fd and setting up pcb's file pointer. normal fd is between 1 to 7
            if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[2];
            else return -1;
        } 

        // if the dentry is directory file then set up the file pointer to directory
        else if (dentry.file_type == 1) {
            fd = directory_open(filename);

            // sanity check for fd and setting up pcb's file pointer. normal fd is between 1 to 7
            if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[4];
            else return -1;
        } 

        // if the dentry is a ormal file then set up the table pointer to file system
        else {
            fd = fs_open(filename);

            // sanity check for fd and setting up pcb's file pointer. normal fd is between 1 to 7
            if (fd > 1 && fd < 8) pcb->process_file_array[fd].table_ptr = (void *) &file_ops_table[3];
            else return -1;
        }
    }

    return fd;
}

/* 
 * system_close
 *   DESCRIPTION: calls the relevant close function
 *   INPUTS: fd - file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: the output of the relevant close function
 *   SIDE EFFECTS: none
 */
int32_t system_close (uint32_t fd) {   
    if (fd < 0 || fd > 7) return -1;
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);


    // check if the file is in use. if it is not then we cannot close it
    if (pcb->process_file_array[fd].flags == 0){
        return -1;
    }

    file_ops_t sys_func_table = *((file_ops_t*)(pcb->process_file_array[fd].table_ptr));
    return sys_func_table.system_close(fd);
}

/* 
 * system_dummy
 *   DESCRIPTION: dummy function to set up jump table in accordance with arguments in syscall_linkage.S
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 (useless as it is never called)
 *   SIDE EFFECTS: none
 */
int32_t system_dummy (int32_t dummy) {
    return 0;
}

/* 
 * ret_fail
 *   DESCRIPTION: returns -1 for stdin and stdout functions that are irrelevant
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: -1 
 *   SIDE EFFECTS: none
 */
int32_t ret_fail () {
    return -1;
}
