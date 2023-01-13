#include "i8259.h"
#include "lib.h"

/* initialize the piano */
void piano_handler_init();

/* handles piano on int. */
void piano_handler(int scan_code);
