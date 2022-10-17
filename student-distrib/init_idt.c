#include "x86_desc.h"
#include "lib.h"
#include "init_idt.h"
#include "driver_linkage.h"

void initIdtFunc();

void divideerror() {
    clear();
    printf(" There is a divide error");
    while(1){}
}

void singleStepInterrupt() {
    clear();
    printf(" There is a single step error");
    while(1){}
}

void nmi() {
    clear();
    printf(" Non maskable interrupt");
    while(1){}
}

void breakpoint() {
    clear();
    printf(" There is a breakpoint error");
    while(1){}
}

void overflow() {
    clear();
    printf(" Overflow error");
    while(1){}
}

void bound_range() {
    clear();
    printf(" Bound Range Exceeded");
    while(1){}
}

void invalidopcode() {
    clear();
    printf(" Invalid Opcode Error");
    while(1){}
}

void CoprocessorNotAvailable() {
    clear();
    printf(" Coprocessor is not available");
    while(1){}
}

void DoubleFault() {
    clear();
    printf(" Double Fault");
    while(1){}
}

void CoprocessorSegmentOverrun() {
    clear();
    printf(" Coprocessor Segment Overrun");
    while(1){}
}

void InvalidTaskStateSegment() {
    clear();
    printf(" Invalid Task State Segment");
    while(1){}
}

void Segmentnotpresent() {
    clear();
    printf(" Segment not present");
    while(1){}
}

void StackSegmentFault() {
    clear();
    printf(" Stack Segment Fault");
    while(1){}
}

void GeneralProtectionFault() {
    clear();
    printf(" General Protection Fault");
    while(1){}
}

void PageFault() {
    clear();
    printf(" Page Fault");
    while(1){}
}

void x87FloatingPointException() {
    clear();
    printf(" x87 Floating Point Exception");
    while(1){}
}

void AlignmentCheck() {
    clear();
    printf(" Alignment Check");
    while(1){}
}

void MachineCheck() {
    clear();
    printf(" Machine Check");
    while(1){}
}

void SIMDFloatingPointException() {
    clear();
    printf(" SIMD Floating-Point Exception");
    while(1){}
}

void VirtualizationException() {
    clear();
    printf(" Virtualization Exception");
    while(1){}
}

void ControlProtectionException() {
    clear();
    printf(" Control Protection Exception");
    while(1){}
}

void initIdtFunc()
{
    int i;
    for(i=0; i<NUM_VEC; i++) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved3 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].dpl = 0x0;
        idt[i].present = 0;
        if (i == 128) idt[i].dpl = 0x3;
        if (i <= 21 || i == 33) idt[i].present = 1;
    }

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
    SET_IDT_ENTRY(idt[33], keyboard_driver_assembly);
}
