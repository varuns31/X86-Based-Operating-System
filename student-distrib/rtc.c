#include "rtc.h"

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

volatile unsigned int rtc_flag = 0; 

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
    rate = 6;


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

    rtc_flag = 1;

    // call test case
    // test_interrupts();
    
    send_eoi(IRQ_LINE_RTC);

    return; 
}


/* 
 * rtc_read
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    while(!rtc_flag);
    rtc_flag = 0;
    return 0;
}

/* 
 * rtc_write
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes){

    // check if buffer it 4 bytes
    if(sizeof(buf) != sizeof(uint32_t))
        return -1;

    // get frequence from the buffer
    unsigned int frequency = *((unsigned int*)buf);
    
    // check if frequency within boounds and a power of 2
    if(frequency > 1024 || frequency < 2  || frequency % 2 != 0)
        return -1;

    // The formula to calculate the interrupt rate is:
    //   freq. = 32768 >> (rate-1)

    unsigned int rate = 1;

    unsigned int freq_count = 32768;

    while(freq_count != frequency){
        rate++;
        freq_count >>= 1;
    }

    rtc_handler_set_rate(rate);

    return 0;
}


/* 
 * rtc_open
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t rtc_open (const uint8_t* filename){
    if(filename == NULL)
        return -1;

    rtc_handler_set_rate(DEFAULT_RTC_RATE);
    return 0;
}

/* 
 * rtc_close
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int32_t close (int32_t fd){
    return 0;
}

