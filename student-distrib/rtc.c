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


/* 
 * rtc_handler_init
 *   DESCRIPTION: Handles the RTC initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
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

/* 
 * rtc_handler
 *   DESCRIPTION: interrupt handler for RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void rtc_handler() {

    // Register C
    // setup register C
    outb(REG_C_IDX, RTC_PORT_ADDR);
    
    // flush the current value
    inb(RTC_PORT_DATA);

    // call test case
    test_interrupts();
    
    send_eoi(IRQ_LINE_RTC);

    return; 
}
