#include "rtc.h"

void rtc_handler_init() {
    char prev;
    unsigned int rate;

    enable_irq(0x08);


    // Register B
    outb( 0x8B, 0x70);		    // select register B, and disable NMI
    prev = inb(0x71);	    // read the current value of register B
    outb( 0x8B, 0x70);		    // set the index again (a read will reset the index to register D)
    outb( prev | 0x40, 0x71);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    

    rate = 6;			// rate is 1024

    // Register A
    outb( 0x8A, 0x70);		// set index to register A, disable NMI
    prev = inb(0x71);	// get initial value of register A
    outb( 0x8A, 0x70);		// reset index to A
    outb( (prev & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.

    return; 
}

void rtc_handler() {

    // Register C
    outb( 0x0C, 0x70);	// select register C
    inb(0x71);		// just throw away contents
    test_interrupts();
    send_eoi(0x08);
    return; 
}
