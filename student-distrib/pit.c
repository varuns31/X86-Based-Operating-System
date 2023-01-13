#include "pcb.h"
#include "lib.h"
#include "driver_linkage.h"
#include "i8259.h"
#include "paging.h"
#include "x86_desc.h"
#include "ret_to_user.h"
#include "syscall.h"
#include "pit.h"

#define MAX_PIT_FREQ 1193180
#define CURR_PIT_FREQ 100
#define PIT_INTERRUPT_LINE 0

int pit_curr_terminal;
int open_terminals;
int prev_ebp, prev_esp;
int curr_esp0;
int prev_pit_terminal;
/* 
 * pit_init
 *   DESCRIPTION: Handles the PIT initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void pit_init () {
    enable_irq(PIT_INTERRUPT_LINE);
    int divisor;
    printf("reached pit init%d\n", piano);
    if(piano == 1) {
        divisor = MAX_PIT_FREQ / 10000;   /* Calculate our divisor */
    } else {
        divisor = MAX_PIT_FREQ / CURR_PIT_FREQ;   /* Calculate our divisor */
    }
    outb(0x36, 0x43);                             /* Set our command byte 0x36 */
    outb(divisor & 0xFF, 0x40);                   /* Set low byte of divisor */
    outb(divisor >> 8, 0x40);                     /* Set high byte of divisor */

    //initialise global variables for use
    open_terminals = 0;
    pit_curr_terminal = 0;
}

/* 
 * pit_handler
 *   DESCRIPTION: interrupt handler for PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Gives processing to next terminal
 */
void pit_handler () {
    // printf("reached pit handler %d\n", piano);
    if (piano == 1){
        send_eoi(0);
        return;
    }

    cli();
    open_terminals++;
    curr_esp0 = tss.esp0;

    //Save Ebp and Esp values to global variables
    asm volatile("                \n\
            mov %%ebp, %0          \n\
            "
            : "=r"(prev_ebp)
    );

    asm volatile("                \n\
            mov %%esp, %0          \n\
            "
            : "=r"(prev_esp)
    );

    //Opening Shells for the first time in all terminals
    if(open_terminals <= 3){
        if(open_terminals != 1) {
            pcb_t* switch_pcb = (pcb_t *) find_pcb(terminals[open_terminals - 1].latest_pid);
            switch_pcb -> saved_ebp = prev_ebp;
            switch_pcb -> saved_esp = prev_esp; 
            terminals[open_terminals - 1].esp0 = curr_esp0;        
        }

        pit_curr_terminal++;
        switch_terminal(open_terminals);
        send_eoi(PIT_INTERRUPT_LINE);
        system_execute((uint8_t*) "shell"); 
        sti();
        return;
        
    } else if(open_terminals == 4){
        pit_curr_terminal = 1;
        switch_terminal(pit_curr_terminal);
        pit_switch_terminal();
        return;
    }

    prev_pit_terminal = pit_curr_terminal;
    pit_curr_terminal++;
    if (pit_curr_terminal % 4 == 0) pit_curr_terminal = 1;

    //Switch to next terminal for processing round robin
    pit_switch_terminal();
    return;
}



/* 
 * pit_switch_terminal
 *   DESCRIPTION: interrupt handler for PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Switches stack to user process of next terminal
 */
void pit_switch_terminal(){ 
    //Get PCB to switch into
    pcb_t* switch_pcb = (pcb_t *) find_pcb(curr_pid);
    switch_pcb -> saved_ebp = prev_ebp;
    switch_pcb -> saved_esp = prev_esp; 
    terminals[prev_pit_terminal].esp0 = curr_esp0;

    curr_pid = terminals[pit_curr_terminal].latest_pid;

    pcb_t* curr_pcb = (pcb_t *) find_pcb(curr_pid);
    int curr_ebp = curr_pcb -> saved_ebp;
    int curr_esp = curr_pcb -> saved_esp;

    // set the tss esp and ss. Note that esp0 points to the next memory address
    tss.ss0 = KERNEL_DS;
    // calculating the addr w.r.t. 8 MB (given in the docs)
    tss.esp0 = terminals[pit_curr_terminal].esp0;

    // Changing paging according to next terminal
    schedule_paging(pit_curr_terminal);

    send_eoi(PIT_INTERRUPT_LINE);

    //restore ebp esp of next terminal
    asm volatile("mov %0,%%esp \n\
                 mov %1,%%ebp \n\
            "
            :
            :"r"(curr_esp),"r"(curr_ebp)
            :"memory","cc","esp","ebp"
            );
}

 //Play sound using built in speaker
void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
    outb(0xb6, 0x43);
 	outb((uint8_t) (Div) , 0x42);
 	outb((uint8_t) (Div >> 8), 0x42);
 
    //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(tmp | 3, 0x61);
 	}
 }
 
 //make it shutup
void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
 
 	outb(tmp, 0x61);
 }

  //Make a beep
 void beep() {
 	 play_sound(256);
 	 timer_wait(100);
 	 nosound();
    //  timer_wait(10);
          //set_PIT_2(old_frequency);
 }

void note(int freq, int time) {
 	 play_sound(freq);
 	 timer_wait(time);
 	 nosound();
    //  timer_wait(1);
          //set_PIT_2(old_frequency);
 }



//Play sound using built in speaker
void timer_wait(int count) {
 	uint32_t Div;
 	unsigned int tmp;
 
    //Set the PIT to the desired frequency
 	Div = 1193180;

    tmp = Div * count;

    while(tmp > 0){
        tmp--;
    }
 }