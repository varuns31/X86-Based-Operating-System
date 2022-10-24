#include "i8259.h"
#include "lib.h"

void keyboard_handler_init();
void keyboard_handler();
int32_t terminal_open (const uint8_t* filename);
int32_t terminal_close (int32_t fd);
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
