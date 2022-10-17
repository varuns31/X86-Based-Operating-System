#include "keyboard.h"

void keyboard_handler_init() {
    enable_irq(1);
    return;
}

void keyboard_handler() {
    clear();
    int a = inb(0x60);
    printf(" The key pressed is %d\n", a);
    send_eoi(0x01);
    return;
}
