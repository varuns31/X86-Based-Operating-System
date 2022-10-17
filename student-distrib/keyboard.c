#include "keyboard.h"

#define IRQ_LINE_KEYBOARD 0x01
#define KEYBOARD_PORT_ADDR 0x60

// create the keyboard map
char key_map[128] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '-', '\b',  
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\',
    '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\r',
    '\0', '\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0'
};


/* 
 * keyboard_handler_init
 *   DESCRIPTION: Handles the keyboard initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void keyboard_handler_init() {
    // enable interrupts
    enable_irq(IRQ_LINE_KEYBOARD);
    // clear the screen
    clear();
    return; 
}

/* 
 * keyboard_handler
 *   DESCRIPTION: interrupt handler for keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void keyboard_handler() {
    int scan_code = inb(KEYBOARD_PORT_ADDR);
    // 0x4d is the upper bound for preventing the key up scancode
    if(scan_code > 0x4d) {
        // send eoi if scancode out of bounds
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // write character to the screen
    putc(key_map[scan_code]);

    // send eoi once done
    send_eoi(IRQ_LINE_KEYBOARD);

    return;
}
