#ifndef _RETF_H
#define _RETF_H
#ifndef ASM

// define asembly linkage functions
extern void jump_usermode();
extern void pit_switch();

// to get the program eip which is 4 bytes by the docs
extern uint8_t prog_eip[4];

#endif /* ASM */
#endif /* _RETF_H */
