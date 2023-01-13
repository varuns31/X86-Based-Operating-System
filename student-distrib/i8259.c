/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

# define PIC_OFFSET 8
# define PIC_PORT_IDX 2
# define NEXT_PORT 1

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* 
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void i8259_init(void) {

    // init cached values
    master_mask = 0xff;
    slave_mask = 0xff;

    // commands for master
    
    // initialize PIC
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + NEXT_PORT);

    // Setting cascaded configuration using next port
    outb(ICW3_MASTER, MASTER_8259_PORT + NEXT_PORT);
    outb(ICW4, MASTER_8259_PORT + NEXT_PORT);

    // commands for slave
    
    // initialize PIC
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + NEXT_PORT);

    // Setting cascaded configuration using next port
    outb(ICW3_SLAVE, SLAVE_8259_PORT + NEXT_PORT);
    outb(ICW4, SLAVE_8259_PORT + NEXT_PORT);

    // push cached values
    // setting mask for master
    outb(master_mask, MASTER_8259_PORT + NEXT_PORT);

    // setting mask for slave
    outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);
    
    // enable secondary pic
    enable_irq(PIC_PORT_IDX);

    return;
}

/* 
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: irq_num - request line number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void enable_irq(uint32_t irq_num) {

    if(irq_num < PIC_OFFSET) {
        // since 1 bit to be operated
        master_mask &= ~(0x01 << irq_num);
        
        // accessing data port
        outb(master_mask, MASTER_8259_PORT + NEXT_PORT); 
        return;
    }

    irq_num -= PIC_OFFSET;

    // since 1 bit to be operated
    slave_mask &= ~(0x01 << irq_num);

    // Accessing data port
    outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);
    
    return;
}

/* 
 * disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: irq_num - request line number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void disable_irq(uint32_t irq_num) {
    
    if(irq_num < PIC_OFFSET) {
        // since 1 bit to be operated
        master_mask |= 0x01 << irq_num;

        // Accessing data port
        outb(master_mask, MASTER_8259_PORT + NEXT_PORT); 
        return;
    }

    irq_num -= PIC_OFFSET;
    // since 1 bit to be operated

    slave_mask |= 0x01 << irq_num;
    // Accessing data port
    outb(slave_mask, SLAVE_8259_PORT + NEXT_PORT);
    
    return;
}

/* 
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num - request line number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void send_eoi(uint32_t irq_num) {

    if(irq_num < PIC_OFFSET) {
        outb(EOI | irq_num, MASTER_8259_PORT);
        return;
    }

    irq_num -= PIC_OFFSET;
    outb(EOI | irq_num, SLAVE_8259_PORT);
    outb(EOI | PIC_PORT_IDX, MASTER_8259_PORT);

    return;
}

