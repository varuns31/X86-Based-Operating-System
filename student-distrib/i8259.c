/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    // init cached values
    master_mask = 0xff;
    slave_mask = 0xff;

    // commands for master
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + 0x01);
    outb(ICW3_MASTER, MASTER_8259_PORT + 0x01);
    outb(ICW4, MASTER_8259_PORT + 0x01);

    // commands for slave
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 0x01);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 0x01);
    outb(ICW4, SLAVE_8259_PORT + 0x01);

    // push cached values
    outb(master_mask, MASTER_8259_PORT + 0x01);
    outb(slave_mask, SLAVE_8259_PORT + 0x01);

    return;
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

    if(irq_num < 8) {
        master_mask &= ~(0x01 << irq_num);
        outb(master_mask, MASTER_8259_PORT + 0x01); 
        return;
    }

    irq_num -= 8;
    slave_mask &= ~(0x01 << irq_num);
    outb(slave_mask, SLAVE_8259_PORT + 0x01);
    
    return;
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    
    if(irq_num < 8) {
        master_mask |= 0x01 << irq_num;
        outb(master_mask, MASTER_8259_PORT + 0x01); 
        return;
    }

    irq_num -= 8;
    slave_mask |= 0x01 << irq_num;
    outb(slave_mask, SLAVE_8259_PORT + 0x01);
    
    return;
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    if(irq_num < 8) {
        outb(EOI | irq_num, MASTER_8259_PORT);
        return;
    }

    irq_num -= 8;
    outb(EOI | 2, MASTER_8259_PORT);
    outb(EOI | irq_num, SLAVE_8259_PORT);

    return;
}
