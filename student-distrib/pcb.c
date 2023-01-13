#include "pcb.h"
#include "lib.h"

// helpful constants
#define ONE_MB 1024 * 1024
#define ONE_KB 1024

// by the docs
#define MAX_PROCESSES 6

// by the docs
const int kernel_stack_base_addr = 8 * ONE_MB;

/* 
 * find_pcb
 *   DESCRIPTION: Handles the keyboard initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
int32_t find_pcb (int pid) {
    // gets you the starting address of pcb for current kernel block
    return (8 * ONE_MB) - (8 * ONE_KB * (pid + 1));
}  

/* 
 * init_pcbs
 *   DESCRIPTION: inits the PCB
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void init_pcbs() {
    pcb_t empty_pcb;
    empty_pcb.parent_id = -1;
    empty_pcb.saved_esp = 0;
    empty_pcb.saved_ebp = 0;
    empty_pcb.active = 0; 

    int i = 0;
    
    for(i = 1; i < 7; i++) {
        empty_pcb.process_id = i - 1;
        // gets you the starting address of pcb for current kernel block (8MB - some offset)
        int* addr_to_copy = (int *) (kernel_stack_base_addr - (8 * ONE_KB * i));
        memcpy((void *) addr_to_copy, (void*) &empty_pcb, sizeof(pcb_t));
    }

    // initialize the argument buffer
    for(i = 0; i < 128; i++) {
        empty_pcb.pcb_arg[i] = 0;
    }

}

/* 
 * get_first_inactive_pid
 *   DESCRIPTION: returns the first inactive pid
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pid or -1 if not found
 *   SIDE EFFECTS: none
 */
int32_t get_first_inactive_pid() {

    int i = 0;
    // 10 is for testing purposes for this checkpoint only!
    for(i = 0; i < 6; i++) {
        // gets you the starting address of pcb for current kernel block (8MB - some offset)
        pcb_t * temp_pcb = (pcb_t *) (kernel_stack_base_addr - (8 * ONE_KB * (i+1)));
        if(temp_pcb->active == 0){
            return i;
        }
    }
    return -1;
}

/* 
 * set_pcb_by_pid
 *   DESCRIPTION: Copies PCB data at the respective PID location
 *   INPUTS: PID and the ptr to PCB
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes PCB values in kernel stack
 */
void set_pcb_by_pid(pcb_t * cur_pcb, int pid) {
    // gets you the starting address of pcb for current kernel block (8MB - some offset)
    pcb_t * addr_to_copy = (pcb_t *) (kernel_stack_base_addr - (8 * ONE_KB * (pid + 1)));
    memcpy((void *) addr_to_copy, (void*) cur_pcb, sizeof(pcb_t));
}

/* 
 * get_pcb_by_pid
 *   DESCRIPTION: Returns the PCB data into a provided PCB ptr from the desired PID
 *   INPUTS: PCB ptr to fetch val and PID
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies to provided PCB ptr
 */
void get_pcb_by_pid(pcb_t * cur_pcb, int pid) {
    // gets you the starting address of pcb for current kernel block (8MB - some offset)
    pcb_t * addr_to_copy = (pcb_t *) (kernel_stack_base_addr - (8 * ONE_KB * (pid + 1)));
    memcpy((void *) cur_pcb, (void*) addr_to_copy, sizeof(pcb_t));
}
