#include "keyboard.h"

#define IRQ_LINE_KEYBOARD 0x01
#define KEYBOARD_PORT_ADDR 0x60

// create the keyboard map
char key_map[2][128] = {
    {
        'x', 'x', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'x',  
        'x', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        'x', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'x',
        'x', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 'x'
    },
    {
        'x', 'x', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 'x',  
        'x', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        'x', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 'x',
        'x', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 'x'
    }
};

struct keyboard_mapper {
    int caps_lock;
    int shift;
    int ctrl;
    int alt;
} key_tracker;

int cur_line_counter = 1;

int is_alpha(int scan_code);

int is_alpha(int scan_code) {
    if( 
        (scan_code > 15 && scan_code < 26) || 
        (scan_code > 29 && scan_code < 39) ||
        (scan_code > 43 && scan_code < 51)
    ) return 1;
    
    return 0;
}

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
    
    key_tracker.caps_lock = 0;
    key_tracker.shift = 0;
    key_tracker.ctrl = 0;
    key_tracker.alt = 0;
    
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
    
    // 0x80 is the upper bound for preventing the key up scancode
    if(scan_code > 0x80) {

        if(scan_code == 170 || scan_code == 182) key_tracker.shift = 0;
        if(scan_code == 184) key_tracker.alt = 0;
        if(scan_code == 157) key_tracker.ctrl = 0;
        
        // send eoi if scancode out of bounds
        send_eoi(IRQ_LINE_KEYBOARD);
        
        return;
    }

    // shift
    if(scan_code == 42 || scan_code == 54) {
        key_tracker.shift = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // capslock
    if(scan_code == 58) {
        key_tracker.caps_lock = (key_tracker.caps_lock) == 1 ? 0 : 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // alt l and r
    if(scan_code == 56) {
        key_tracker.alt = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // ctrl l and r
    if(scan_code == 29) {
        key_tracker.ctrl = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(key_tracker.ctrl && scan_code == 38) {
        clear();
        set_screen(0, 0);
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(scan_code == 14) {
        int x = get_screen_x();
        int y = get_screen_y();
        int linear = y * 80 + x;
        linear--;
        
        if(linear < 0) {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;  
        }

        set_screen(linear % 80, linear / 80);
        putc(' ');
        x = get_screen_x();
        y = get_screen_y();
        linear = y * 80 + x;
        linear--;
        set_screen(linear % 80, linear / 80);
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(scan_code == 28) {
        cur_line_counter++;
    }

    char to_print;
    int caps_idx = key_tracker.shift ^ key_tracker.caps_lock;
    if(!is_alpha(scan_code)) caps_idx = key_tracker.shift;
    to_print = key_map[caps_idx][scan_code];

    // if(cur_line_counter >= 25) {
    //     clear();
    //     cur_line_counter = 0;
    // }
    // write character to the screen
    putc(to_print);

    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}
