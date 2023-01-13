
#ifndef PCB_H
#define PCB_H

#include "lib.h"
#include "fs.h"

#define MAX_BUFFER_SIZE 128
#define HISTORY_NUM 100

typedef struct __attribute__ ((packed))
{
    unsigned int process_id; // Current process's pid 
    unsigned int parent_id; // Parent Pid for Halt
    unsigned int saved_esp; // Saved Esp for restoring stack when switching processes
    unsigned int saved_ebp; // Saved Ebp for restoring stack when switching processes
    unsigned int base_esp; // Saved Esp for restoring stack when halting processes as base shell esp,ebp gets changed
    unsigned int base_ebp; // Saved Ebp for restoring stack when halting processes as base shell esp,ebp gets changed
    int active; // Is Process active
    fd_node process_file_array[8]; // File array for a process (MAX FILES = (STDIN,STDOUT) + 6)
    char pcb_arg[128]; //buffer argument for get args system call
} pcb_t;

typedef struct __attribute__ ((packed))
{
    unsigned int latest_pid;
    unsigned int screen_x;
    unsigned int screen_y;
    unsigned int esp0;
    unsigned int ss0;
    char buf[128];
    int curr_buff_length;
    int prev_curr_buff_length;
    volatile int enter_pressed;
    volatile int rtc_freq;
    int rtc_fd;
    volatile int rtc_counter;
    volatile char history[HISTORY_NUM][MAX_BUFFER_SIZE];
    volatile int curr_buf_num;
    volatile int latest_buf_num;
    int attribute;
    int og;
} terminal_t;

int curr_pid;

terminal_t terminals[4];

/* inits the FD array */
extern void process_array_init();

/* returns a ptr to the pcb that is required based on pid */
extern int32_t find_pcb (int pid);

#endif /* PCB_H */
