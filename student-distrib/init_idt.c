#include "x86_desc.h"
#include "lib.h"
#include "init_idt.h"
#include "driver_linkage.h"
#include "syscall_linkage.h"
#include "syscall.h"
#include "pcb.h"
#include "pit.h"

# define SYSTEM_HALT_FLAG 69

 /* divideerror
 *   DESCRIPTION: exception handler for Divide by Zero Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void divideerror() {
    printf("There is a divide error\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* singleStepInterrupt
 *   DESCRIPTION: exception handler for singleStepInterrupt Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void singleStepInterrupt() {
    printf("There is a single step error\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* nmi
 *   DESCRIPTION: exception handler for nmi Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void nmi() {
    printf("Non maskable interrupt\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* breakpoint
 *   DESCRIPTION: exception handler for breakpoint Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void breakpoint() {
    printf("There is a breakpoint error\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* overflow
 *   DESCRIPTION: exception handler for overflow Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void overflow() {
    printf("Overflow error\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* bound_range
 *   DESCRIPTION: exception handler for bound_range Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void bound_range() {
    printf("Bound Range Exceeded\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* invalidopcode
 *   DESCRIPTION: exception handler for invalidopcode Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void invalidopcode() {
    printf("Invalid Opcode Error\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* CoprocessorNotAvailable
 *   DESCRIPTION: exception handler for CoprocessorNotAvailable Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void CoprocessorNotAvailable() {
    printf("Coprocessor is not available\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* DoubleFault
 *   DESCRIPTION: exception handler for DoubleFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void DoubleFault() {
    printf("Double Fault\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* CoprocessorSegmentOverrun
 *   DESCRIPTION: exception handler for CoprocessorSegmentOverrun Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void CoprocessorSegmentOverrun() {
    printf("Coprocessor Segment Overrun\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* InvalidTaskStateSegment
 *   DESCRIPTION: exception handler for InvalidTaskStateSegment Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void InvalidTaskStateSegment() {
    printf("Invalid Task State Segment\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* Segmentnotpresent
 *   DESCRIPTION: exception handler for Segmentnotpresent Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void Segmentnotpresent() {
    printf("Segment not present\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* StackSegmentFault
 *   DESCRIPTION: exception handler for StackSegmentFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void StackSegmentFault() {
    printf("Stack Segment Fault\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* GeneralProtectionFault
 *   DESCRIPTION: exception handler for GeneralProtectionFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void GeneralProtectionFault() {
    printf("General Protection Fault\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* PageFault
 *   DESCRIPTION: exception handler for PageFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void PageFault() {
    change_uvid_putc (cur_terminal, cur_terminal);
    printf("Page Fault or a YOU FAULT HAHA!\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* x87FloatingPointException
 *   DESCRIPTION: exception handler for x87FloatingPointException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void x87FloatingPointException() {
    printf("x87 Floating Point Exception\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* AlignmentCheck
 *   DESCRIPTION: exception handler for AlignmentCheck Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void AlignmentCheck() {
    printf("Alignment Check\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* MachineCheck
 *   DESCRIPTION: exception handler for MachineCheck Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void MachineCheck() {
    printf("Machine Check\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* SIMDFloatingPointException
 *   DESCRIPTION: exception handler for SIMDFloatingPointException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void SIMDFloatingPointException() {
    printf("SIMD Floating-Point Exception\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* VirtualizationException
 *   DESCRIPTION: exception handler for VirtualizationException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void VirtualizationException() {
    printf("Virtualization Exception\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* ControlProtectionException
 *   DESCRIPTION: exception handler for ControlProtectionException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void ControlProtectionException() {
    printf("Control Protection Exception\n");
    /* SYSTEM_HALT_FLAG is a number decided by us to indicate the program was terminated by an exception */
    system_halt(SYSTEM_HALT_FLAG);
}

/* initIdtFunc
 *   DESCRIPTION: Initialisation of IDT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets functions in IDT for respective exceptions and handlers 
 */
void initIdtFunc()
{
    int i;
    for(i=0; i<NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved3 = 1;               //Interrupt Gate Set
        idt[i].reserved2 = 1;               //Interrupt Gate Set
        idt[i].reserved1 = 1;               //Interrupt Gate Set
        idt[i].reserved0 = 0;               //Interrupt Gate Set
        idt[i].size = 1;                    //32-bit values
        idt[i].dpl = 0x0;                   //Only Kernel Access
        idt[i].present = 0;                 //Not Present
        if (i == 128) idt[i].dpl = 0x3;     //System call can be called by user

        //exceptions, keyboard, rtc, pit, system calls present 
        if (i <= 21 || i == 33 || i == 40 || i == 32 || i == 128 || i == 44) idt[i].present = 1;  

    }

    // Exceptions filled from 0-21
    SET_IDT_ENTRY(idt[0], divideerror);
    SET_IDT_ENTRY(idt[1], singleStepInterrupt);
    SET_IDT_ENTRY(idt[2], nmi);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], bound_range);
    SET_IDT_ENTRY(idt[6], invalidopcode);
    SET_IDT_ENTRY(idt[7], CoprocessorNotAvailable);
    SET_IDT_ENTRY(idt[8], DoubleFault);
    SET_IDT_ENTRY(idt[9], CoprocessorSegmentOverrun);
    SET_IDT_ENTRY(idt[10], InvalidTaskStateSegment);
    SET_IDT_ENTRY(idt[11], Segmentnotpresent);
    SET_IDT_ENTRY(idt[12], StackSegmentFault);
    SET_IDT_ENTRY(idt[13], GeneralProtectionFault);
    SET_IDT_ENTRY(idt[14], PageFault);
    SET_IDT_ENTRY(idt[16], x87FloatingPointException);
    SET_IDT_ENTRY(idt[17], AlignmentCheck);
    SET_IDT_ENTRY(idt[18], MachineCheck);
    SET_IDT_ENTRY(idt[19], SIMDFloatingPointException);
    SET_IDT_ENTRY(idt[20], VirtualizationException);
    SET_IDT_ENTRY(idt[21], ControlProtectionException);

    // by the docs
    SET_IDT_ENTRY(idt[32], pit_driver_linkage);
    SET_IDT_ENTRY(idt[33], keyboard_driver_linkage);
    SET_IDT_ENTRY(idt[40], rtc_driver_linkage);
    SET_IDT_ENTRY(idt[44], mouse_driver_linkage);
    
    // by the docs
    SET_IDT_ENTRY(idt[128], syscall_linkage);
}

