#include "i8259.h"
#include "lib.h"



void rtc_handler_init();
void rtc_handler();

void rtc_handler_set_rate(unsigned int rate);

int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_open (const uint8_t* filename);

int32_t rtc_close (int32_t fd);
