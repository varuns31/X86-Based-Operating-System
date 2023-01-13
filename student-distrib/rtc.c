#include "rtc.h"
#include "pcb.h"
#include "pit.h"
#include "pcb.h"

// the interrupt line mentioned by docs
#define IRQ_LINE_RTC 0x08
// the address of the RTC port mentioned by docs
#define RTC_PORT_ADDR 0x70
// the data RTC port mentioned by docs
#define RTC_PORT_DATA 0x71

// the register A offset mentioned by docs
#define REG_A_IDX 0x8A
// the register B offset mentioned by docs
#define REG_B_IDX 0x8B
// the register C offset mentioned by docs
#define REG_C_IDX 0x8C

// rtc constants
#define DEFAULT_RTC_RATE 15
#define MAX_FREQ 32768
#define MAX_QEMU_FREQ 1024
#define MIN_FREQ 2

#define COUNTER_MULTIPLIER 6.5 //6.5 is a magic number which we get because there are 3 terminals and pit handler makes the program slower

volatile unsigned int rtc_flag = 0; 

int max_counter;

/* 
 * rtc_handler_init
 *   DESCRIPTION: Handles the RTC initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */

 // register A - rate divider rule
 // register B - disbale NMI (non maskable interrupts)
 // register C - upon a IRQ 8, Status Register C will contain a bitmask telling which interrupt happened.
 //            - What is important is that if register C is not read after an IRQ 8, then the interrupt 
 //            - will not happen again. So, even if you don't care about what type of interrupt it is, 
 //            - just attach this code to the bottom of your IRQ handler to be sure you get another interrupt.
void rtc_handler_init() {
    char init_val;
    unsigned int rate;

    enable_irq(IRQ_LINE_RTC);

    // Register B setup
    // setup register B
    outb(REG_B_IDX, RTC_PORT_ADDR);

    // get value of register B
    init_val = inb(RTC_PORT_DATA);

    // set register B index again
    outb(REG_B_IDX, RTC_PORT_ADDR);

    // turn on bit 6 (0x40) of register B 
    outb(init_val | 0x40, RTC_PORT_DATA);


    // rate is 1024 (2^(10-4))
    //max rate is 15 from OSdev for rtc virtualization.
    rate = 5;

    // Register A setup
    // set register A index again
    outb(REG_A_IDX, RTC_PORT_ADDR);

    // get value of register A
    init_val = inb(RTC_PORT_DATA);	

    // set register A index again
    outb(REG_A_IDX, RTC_PORT_ADDR);		

    // set botton 4 bits to rate
    outb((init_val & 0xF0) | rate, RTC_PORT_DATA); 

    terminals[pit_curr_terminal].rtc_counter = 0;

    return; 
}


void rtc_handler_set_rate(unsigned int rate) {
    char init_val;

    // Register A setup
    // set register A index again
    outb(REG_A_IDX, RTC_PORT_ADDR);

    // get value of register A
    init_val = inb(RTC_PORT_DATA);	

    // set register A index again
    outb(REG_A_IDX, RTC_PORT_ADDR);		

    // set botton 4 bits to rate
    outb((init_val & 0xF0) | rate, RTC_PORT_DATA); 

    return; 
}


/* 
 * rtc_handler
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
void rtc_handler() {
    // Register C
    // setup register C
    outb(REG_C_IDX, RTC_PORT_ADDR);
    
    // flush the current value
    inb(RTC_PORT_DATA);

    //increment counter for virtualised rtc
    terminals[pit_curr_terminal].rtc_counter += 1;
    
    send_eoi(IRQ_LINE_RTC);

    return; 
}


/* 
 * rtc_read
 *   DESCRIPTION: Waits until an interrupt is fired and then sets the rtc_flag to 0 and return 0
 *   INPUTS: fd - file descriptor for rtc
 *           buf - unused, but it is usually for putting in the read data
 *           nbytes - unused, but number of bytes read
 *   OUTPUTS: none
 *   RETURN VALUE: read always returns a 0
 *   SIDE EFFECTS: none
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    
    while((int)(terminals[pit_curr_terminal].rtc_counter * COUNTER_MULTIPLIER) != max_counter);
    //reset counter for curr terminal
    terminals[pit_curr_terminal].rtc_counter = 0;
    memcpy(buf,(void*)& terminals[pit_curr_terminal].rtc_freq, sizeof(int));
    return 0;
}

/* 
 * rtc_write
 *   DESCRIPTION: sets the rtc frequency to a certain number depending on what is passed in buffer
 *   INPUTS: fd - file descriptor for rtc
 *           buf - first 4 bytes contain the frequency we want to set the rtc to
 *           nbytes - unused, but number of bytes that are written
 *   OUTPUTS: None
 *   RETURN VALUE: write always returns a 0
 *   SIDE EFFECTS: None
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes){

    // check if buffer it 4 bytes
    if(sizeof(buf) != sizeof(uint32_t))
        return -1;

    // get frequence from the buffer
    unsigned int frequency = *((unsigned int*)buf);
    
    // check if frequency within bounds and a power of 2
    if(frequency > MAX_FREQ || frequency < MIN_FREQ || frequency % 2 != 0)
        return -1;

    terminals[pit_curr_terminal].rtc_freq = frequency;
    max_counter = MAX_QEMU_FREQ / frequency;

    return 0;
}


/* 
 * rtc_open
 *   DESCRIPTION: rtc rate changed to default
 *   INPUTS: filename - for rtc
 *   OUTPUTS: none
 *   RETURN VALUE: if the file name is not correct return -1
 *   SIDE EFFECTS: rtc rate changed to default
 */
int32_t rtc_open (const uint8_t* filename){
    
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    int fd_idx;
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        if(pcb->process_file_array[fd_idx].flags == 0){
            // set the file array at specified location
            pcb->process_file_array[fd_idx].inode_num = 0;
            pcb->process_file_array[fd_idx].file_pos = 0;
            pcb->process_file_array[fd_idx].flags = 1; // indicated in use
            //set base values in terminals array for current pit terminal
            terminals[pit_curr_terminal].rtc_fd = fd_idx;
            terminals[pit_curr_terminal].rtc_freq = 0;
            terminals[pit_curr_terminal].rtc_counter = 0;
            
            return fd_idx;
        }
    }
    printf("Error: Too many files are open\n");
    return -1;
}

/* 
 * rtc_close
 *   DESCRIPTION: The close system call closes the specified file descriptor and makes it available for return from later calls to open.
 *   You should not allow the user to close the default descriptors (0 for input and 1 for output). 
 *   INPUTS: fd - file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: Trying to close an invalid descriptor should result in a return value of -1; successful closes should return 0.
 *   SIDE EFFECTS: closes rtc
 */
int32_t rtc_close (int32_t fd){
    if (fd == 0 || fd == 1) return -1;
    //rtc_fd at max frequency means rtc is not open
    terminals[pit_curr_terminal].rtc_fd = MAX_FREQ;
    return 0;
}
