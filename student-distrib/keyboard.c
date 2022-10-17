#include "keyboard.h"

char key_map[50] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '-', '\b',  
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\',
    '~', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\r',
    '~', '~', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '~'
};

void keyboard_handler_init() {
    enable_irq(1);
    clear();
    return;
}

void keyboard_handler() {
    int scan_code = inb(0x60);
    if(a > 0x4d) {
        send_eoi(0x01);
        return;
    }

    putc(key_map[scan_code - 0x02]);
    send_eoi(0x01);

    return;
}
