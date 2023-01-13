#include "paging.h"
#include "lib.h"
#include "pcb.h"
#include "enable_paging.h"
#include "ret_to_user.h"
#include "pit.h"

/* 
 * paging_init
 *   DESCRIPTION: initializes page table and page directory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void paging_init() {
    int i = 0;
    
    // initialize the page table entries
    for(i = 0; i < Num_Entries; i++) {
        page_table[i].present = 0;       
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disable = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].pg_atrr_table = 0;
        page_table[i].global = 0;
        page_table[i].page_addr = i;
    }

    // for video memory inside the page table set it to present
    page_table[Video_memory_loc].present = 1;

    // initialize page directory entries
    for(i = 0; i < Num_Entries; i++) {
        page_directory[i].pde_KB.present = 0;
        page_directory[i].pde_KB.read_write = 1;
        page_directory[i].pde_KB.user_supervisor = 0;
        page_directory[i].pde_KB.write_through = 0;
        page_directory[i].pde_KB.cache_disable = 0;
        page_directory[i].pde_KB.accessed = 0;
        page_directory[i].pde_KB.page_size = 0;
        page_directory[i].pde_KB.available = 0;
        page_directory[i].pde_KB.page_addr = 0x0000; 
    }

    // sets the first entry inside the pdt to point to the 4kb page
    page_directory[0].pde_KB.present = 1;
    page_directory[0].pde_KB.page_addr = (int) (page_table) >> mask12;
    page_directory[0].pde_KB.user_supervisor = 1;
    
    // sets the second entry inside the pdt to point to the 4mb page
    page_directory[1].pde_MB.present = 1;
    page_directory[1].pde_MB.page_size = 1;
    page_directory[1].pde_MB.page_addr = kernel_mem_loc;
    page_directory[1].pde_MB.read_write = 1;
    
    // load the page directory table and than enable paging
    loadPageDirectory(page_directory);
    enablePaging(); 
}

/* 
 * process_paging
 *   DESCRIPTION: page the program image
 *   INPUTS: pid -> id of the process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void process_paging(int pid) {
    flush_tlb();
    // sets the second entry inside the pdt to point to the 4mb page
    page_directory[program_img].pde_MB.present = 1;
    page_directory[program_img].pde_MB.page_size = 1;
    page_directory[program_img].pde_MB.user_supervisor = 1;
    // discussion week 10 slide 7 and by the docs => Physical Addr = 8 MB + (4 * PID) MB
    page_directory[program_img].pde_MB.page_addr = (((8 + (4 * pid)) * ONE_MB)) >> mask12;
    page_directory[program_img].pde_MB.read_write = 1;
    flush_tlb();
    
}

/* 
 * video_paging_from_addr
 *   DESCRIPTION: page the user video memory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void video_paging_from_addr() {
    // sets the second entry inside the pdt to point to the 4mb page

    flush_tlb();

    // sets the first entry inside the pdt to point to the 4kb page
    page_directory[virt_page_idx].pde_KB.present = 1;
    page_directory[virt_page_idx].pde_KB.user_supervisor = 1;
    page_directory[virt_page_idx].pde_KB.page_size = 0; 
    page_directory[virt_page_idx].pde_KB.read_write = 1;


    // user_page_directory->pcd = 1;
    // user_page_directory->ps = 1;

    int i;

    // initialize the page table entries
    for(i = 0; i < Num_Entries; i++) {
        page_table_vidmap[i].present = 0;    
        page_table_vidmap[i].page_addr = i;
        page_table_vidmap[i].read_write = 1;
        page_table_vidmap[i].user_supervisor = 1;
        page_table_vidmap[i].write_through = 0;
        page_table_vidmap[i].cache_disable = 0;
        page_table_vidmap[i].accessed = 0;
        page_table_vidmap[i].dirty = 0;
        page_table_vidmap[i].pg_atrr_table = 0;
        page_table_vidmap[i].global = 0;
    }
    
    //Mark a page table present at 132MB
    page_directory[virt_page_idx].pde_KB.page_addr = (int) (page_table_vidmap) >> mask12;

    //Mark vido memory as present in page table that maps to actual video memory
    page_table_vidmap[Video_memory_loc].present = 1;
    flush_tlb();

}

/* 
 * mult_terminal_paging
 *   DESCRIPTION: Initialises paging for new terminal buffers
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void mult_terminal_paging(){

    //marking terminal buffers as present
    page_table[Video_memory_loc + 1].present = 1;
    page_table[Video_memory_loc + 2].present = 1;
    page_table[Video_memory_loc + 3].present = 1;

    page_table[Video_memory_loc].page_addr = Video_memory_loc + 1;
    flush_tlb();
    clear();

    ATTRIB = 0x4E;
    page_table[Video_memory_loc].page_addr = Video_memory_loc + 2;
    flush_tlb();
    clear();

    ATTRIB = 0x30;
    page_table[Video_memory_loc].page_addr = Video_memory_loc + 3;
    flush_tlb();
    clear();

    ATTRIB = 0x1E;
    page_table[Video_memory_loc].page_addr = Video_memory_loc;
    flush_tlb();
    clear();

    //First terminal after boot is 1
    cur_terminal = 1;   

    int i;

    // 3 terminals + 1 placeholder
    for(i = 0; i < 4; i++) {
        terminals[i].latest_pid = -1;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].curr_buff_length = 0;
        terminals[i].prev_curr_buff_length = 0;
        terminals[i].enter_pressed = 0;
        terminals[i].esp0 = -1;
        terminals[i].ss0 = -1;
        terminals[i].rtc_freq = 0;
        terminals[i].rtc_fd = -1;
        terminals[i].rtc_counter = 0;
        terminals[i].curr_buf_num = 0;
        terminals[i].latest_buf_num = 0;
        terminals[i].og = 0;
    }
    terminals[1].attribute = 0x1E;
    terminals[2].attribute = 0x4E;
    terminals[3].attribute = 0x30;

    flush_tlb();
}

/* 
 * switch_terminal
 *   DESCRIPTION: switches virtual memory to physical memory mapping for terminals and change terminal state
 *   INPUTS: terminal to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void switch_terminal (int terminal) {
    memmove((void *)((Video_memory_loc + cur_terminal) << mask12),(void *) (Video_memory_loc << mask12), PG_Size);
    change_uvid_putc(terminal, pit_curr_terminal);
    save_and_restore_terminal_state(terminal);
    memmove((void *)(Video_memory_loc << mask12),(void *) ((Video_memory_loc + terminal) << mask12), PG_Size);
    //Current terminal is updated
    cur_terminal = terminal;
}

/* 
 * user_video_unmap
 *   DESCRIPTION: Unmap the virtual memory given to user to write to physical video memory directly
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void change_uvid_putc (int terminal, int pit_terminal) {
    if (terminal == pit_terminal){
        page_table_vidmap[Video_memory_loc].page_addr = Video_memory_loc;
        flush_tlb();
        video_mem = (char *)(VIDEO);
    } else {
        page_table_vidmap[Video_memory_loc].page_addr = Video_memory_loc + pit_terminal;
        flush_tlb();
        video_mem = (char *)(VIDEO + PG_Size * pit_terminal);
    }
}

/* 
 * user_video_unmap
 *   DESCRIPTION: Unmap the virtual memory given to user to write to physical video memory directly
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void user_video_unmap() {
    flush_tlb();
    //marking not present
    page_directory[virt_page_idx].pde_KB.present = 0;
    page_table_vidmap[Video_memory_loc].present = 0;
    flush_tlb();
}

/* 
 * clean_page
 *   DESCRIPTION: set the present bit of page directory to 0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void clean_page() {
    //marking not present
    page_directory[program_img].pde_MB.present = 0;
    flush_tlb();
}

/* 
 * schedule_paging
 *   DESCRIPTION: changes program image and changes virtual to physical terminal mapping
 *   INPUTS: pit_terminal to switch
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void schedule_paging(int pit_terminal) {
    process_paging(terminals[pit_terminal].latest_pid);
    change_uvid_putc(cur_terminal, pit_terminal);
}
