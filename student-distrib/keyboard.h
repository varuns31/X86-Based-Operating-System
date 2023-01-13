#include "i8259.h"
#include "lib.h"

/* initialize the keyboard */
void keyboard_handler_init();

/* handles keyboard on int. */
void keyboard_handler();

/* open for terminal driver */
int32_t terminal_open (const uint8_t* filename);

/* close for terminal driver */
int32_t terminal_close (int32_t fd);

/* read for terminal driver */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);

/* write for terminal driver */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);

/* save and restore terminal state */
extern void save_and_restore_terminal_state(int switch_to_terminal);

extern int ATTRIB;
