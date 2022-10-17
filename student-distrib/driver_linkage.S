#define ASM     1
#include "driver_linkage.h"

.text

.globl keyboard_driver_assembly

# rtc_driver_assembly: 
#     pushal
#     pushfl
#     call rtc_handler
#     popfl
#     popal
#     iret

keyboard_driver_assembly:
    pushal
    pushfl
    call keyboard_handler
    popfl
    popal
    sti
    iret

.end
