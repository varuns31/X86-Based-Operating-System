.global jump_usermode
.global pit_switch

jump_usermode:
    # pushing  USER_CS = 0x002B onto the stack
    pushl $0x002B

    # move USER_CS to the data segment register
    movw $0x2B,%ax
    mov %ax, %ds

    # pushing USER_ESP = $0x83FFFFC
    pushl $0x83FFFFC

    # pushing flags
    pushfl
    popl %eax
    
    # set the IO Priviledge field (12-13) to 1 (stI)
    orl $0x200, %eax
    pushl %eax

    # pushing USER_CS = 0x0023
    pushl $0x0023

    # pushing the USER Program's EIP
    pushl prog_eip

    iret