#include "x86_desc.h"
#include "lib.h"
#include "init_idt.h"
#include "driver_linkage.h"

 /* divideerror
 *   DESCRIPTION: exception handler for Divide by Zero Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void divideerror() {
    clear();
    printf(" There is a divide error");
    while(1){}
}

/* singleStepInterrupt
 *   DESCRIPTION: exception handler for singleStepInterrupt Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void singleStepInterrupt() {
    clear();
    printf(" There is a single step error");
    while(1){}
}

/* nmi
 *   DESCRIPTION: exception handler for nmi Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void nmi() {
    clear();
    printf(" Non maskable interrupt");
    while(1){}
}

/* breakpoint
 *   DESCRIPTION: exception handler for breakpoint Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void breakpoint() {
    clear();
    printf(" There is a breakpoint error");
    while(1){}
}

/* overflow
 *   DESCRIPTION: exception handler for overflow Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void overflow() {
    clear();
    printf(" Overflow error");
    while(1){}
}

/* bound_range
 *   DESCRIPTION: exception handler for bound_range Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void bound_range() {
    clear();
    printf(" Bound Range Exceeded");
    while(1){}
}

/* invalidopcode
 *   DESCRIPTION: exception handler for invalidopcode Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void invalidopcode() {
    clear();
    printf(" Invalid Opcode Error");
    while(1){}
}

/* CoprocessorNotAvailable
 *   DESCRIPTION: exception handler for CoprocessorNotAvailable Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void CoprocessorNotAvailable() {
    clear();
    printf(" Coprocessor is not available");
    while(1){}
}

/* DoubleFault
 *   DESCRIPTION: exception handler for DoubleFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void DoubleFault() {
    clear();
    printf(" Double Fault");
    while(1){}
}

/* CoprocessorSegmentOverrun
 *   DESCRIPTION: exception handler for CoprocessorSegmentOverrun Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void CoprocessorSegmentOverrun() {
    clear();
    printf(" Coprocessor Segment Overrun");
    while(1){}
}

/* InvalidTaskStateSegment
 *   DESCRIPTION: exception handler for InvalidTaskStateSegment Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void InvalidTaskStateSegment() {
    clear();
    printf(" Invalid Task State Segment");
    while(1){}
}

/* Segmentnotpresent
 *   DESCRIPTION: exception handler for Segmentnotpresent Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void Segmentnotpresent() {
    clear();
    printf(" Segment not present");
    while(1){}
}

/* StackSegmentFault
 *   DESCRIPTION: exception handler for StackSegmentFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void StackSegmentFault() {
    clear();
    printf(" Stack Segment Fault");
    while(1){}
}

/* GeneralProtectionFault
 *   DESCRIPTION: exception handler for GeneralProtectionFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void GeneralProtectionFault() {
    clear();
    printf(" General Protection Fault");
    while(1){}
}

/* PageFault
 *   DESCRIPTION: exception handler for PageFault Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void PageFault() {
    //clear();
    printf(" Page Fault");
    while(1){}
}

/* x87FloatingPointException
 *   DESCRIPTION: exception handler for x87FloatingPointException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void x87FloatingPointException() {
    clear();
    printf(" x87 Floating Point Exception");
    while(1){}
}

/* AlignmentCheck
 *   DESCRIPTION: exception handler for AlignmentCheck Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void AlignmentCheck() {
    clear();
    printf(" Alignment Check");
    while(1){}
}

/* MachineCheck
 *   DESCRIPTION: exception handler for MachineCheck Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void MachineCheck() {
    clear();
    printf(" Machine Check");
    while(1){}
}

/* SIMDFloatingPointException
 *   DESCRIPTION: exception handler for SIMDFloatingPointException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void SIMDFloatingPointException() {
    clear();
    printf(" SIMD Floating-Point Exception");
    while(1){}
}

/* VirtualizationException
 *   DESCRIPTION: exception handler for VirtualizationException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void VirtualizationException() {
    clear();
    printf(" Virtualization Exception");
    while(1){}
}

/* ControlProtectionException
 *   DESCRIPTION: exception handler for ControlProtectionException Error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Runs infinitely after giving error message
 */
void ControlProtectionException() {
    clear();
    printf(" Control Protection Exception");
    while(1){}
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
        idt[i].reserved3 = 1;//Interrupt Gate Set
        idt[i].reserved2 = 1;//Interrupt Gate Set
        idt[i].reserved1 = 1;//Interrupt Gate Set
        idt[i].reserved0 = 0;//Interrupt Gate Set
        idt[i].size = 1;//32-bit values
        idt[i].dpl = 0x0;//Only Kernel Access
        idt[i].present = 0;//Not Present
        if (i == 128) idt[i].dpl = 0x3;//System call can be called by user
        if (i <= 21 || i == 33 || i == 40) idt[i].present = 1;//exceptions and keyboard and rtc present
    }

    //Exceptions filled from 0-21
    //keyboard at 0x21
    //rtc at 0x28
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
    SET_IDT_ENTRY(idt[33], keyboard_driver_linkage);
    SET_IDT_ENTRY(idt[40], rtc_driver_linkage);
}
