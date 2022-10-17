#include "rtc.h"

#define IRQ_LINE_RTC 0x08
#define RTC_PORT_ADDR 0x70
#define RTC_PORT_DATA 0x71

#define REG_A_IDX 0x8A
#define REG_B_IDX 0x8B
#define REG_C_IDX 0x8C

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
