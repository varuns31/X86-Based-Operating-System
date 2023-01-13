.text

.globl keyboard_driver_linkage, rtc_driver_linkage, pit_driver_linkage, mouse_driver_linkage

# rtc_driver_linkage assembly linkage
rtc_driver_linkage: 
    pushal
    pushfl
    call rtc_handler
    popfl
    popal
    sti
    iret

# keyboard_driver_linkage assembly linkage
keyboard_driver_linkage:
    pushal
    call keyboard_handler
    popal
    iret

# mouse_driver_linkage assembly linkage
mouse_driver_linkage:
    pushal
    call mouse_handler
    popal
    iret 

# pit_driver_linkage assembly linkage
pit_driver_linkage:
    pushal
    pushfl
    call pit_handler   
    popfl
    popal
    sti
    iret

.end

