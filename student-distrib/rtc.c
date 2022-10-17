#include "rtc.h"

void rtc_handler_init() {
    char prev;
    unsigned int rate;

    enable_irq(0x08);

    cli();

    // Register B
    outb(0x70, 0x8B);		    // select register B, and disable NMI
    prev = inb(0x71);	    // read the current value of register B
    outb(0x70, 0x8B);		    // set the index again (a read will reset the index to register D)
    outb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    sti();

    rate = 0x0400;			// rate is 1024

    cli();

    // Register A
    outb(0x70, 0x8A);		// set index to register A, disable NMI
    prev = inb(0x71);	// get initial value of register A
    outb(0x70, 0x8A);		// reset index to A
    outb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    
    sti();

    return; 
}

void rtc_handler() {

    // Register C
    outb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents

    send_eoi(0x01);
    return; 
}
