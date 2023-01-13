#include "i8259.h"
#include "lib.h"

/* inits the RTC driver */
void rtc_handler_init();

/* RTC handler upon int. */
void rtc_handler();

/* RTC helper function to change rate */
void rtc_handler_set_rate(unsigned int rate);

/* reads RTC */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

/* writes RTC */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);

/* opens RTC */
int32_t rtc_open (const uint8_t* filename);

/* closes RTC */
int32_t rtc_close (int32_t fd);

extern int curr_rtc_counter;
