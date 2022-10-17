#include "keyboard.h"

char key_map[128] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '-', '\b',  
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\',
    '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\r',
    '\0', '\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0'
};

void keyboard_handler_init() {
    enable_irq(1);
    clear();
    return; 
}

void keyboard_handler() {
    int scan_code = inb(0x60);
    if(scan_code > 0x4d) {
        send_eoi(0x01);
        return;
    }

    putc(key_map[scan_code]);
    send_eoi(0x01);

    return;
}
