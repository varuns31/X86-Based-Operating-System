#include "x86_desc.h"
#include "lib.h"

#ifndef PAGING_H
#define PAGING_H

#define PG_Size     4096 // page table is 4Kb
#define Num_Entries 1024 // page table and page directory has 1024 entries
#define shell_mem_loc (8 * 1024 * 1024)    

#define VIDEO       0xB8000                             // The physical address of the video
#define mask12 12                                       // gets you the top 20 bits
#define mask20 20                                       // gets you the top 12 bits
#define Video_memory_loc (VIDEO >> mask12)                // Get the top 20 bits of video memory
#define kernel_mem_loc (4 * 1024 * 1024) >> mask12      // Gets you the top 10 bits of video memory as kernel is at 4MB
#define shell_mem_loc (8 * 1024 * 1024)                 // shell memory location is at 8MB. 1024 is 1kb. There are 1024 kb in 1mb. And there are 8mb
#define program_img 32                                  // index 32 in PD, because 128/4 => 32
#define virt_page_idx 33                                // index 33 in PD, because 132/4 => 33  
#define ONE_KB 1024                     /* 1024 bytes */
#define ONE_MB 1024*1024                /* 1024 * 1024 bytes */

int return_to_user_eip;
int return_to_user_esp;


// has all the relevant fields for a 4kb page directory structure
typedef struct __attribute__ ((packed)) {
  uint32_t present         :1;
  uint32_t read_write      :1;
  uint32_t user_supervisor :1;
  uint32_t write_through   :1;
  uint32_t cache_disable   :1;
  uint32_t accessed        :1;
  uint32_t dirty           :1;
  uint32_t page_size       :1;
  uint32_t global          :1;
  uint32_t available       :3;
  uint32_t page_addr       :20;
} pde_4KB_t;

// has all the relevant fields for a 4mb page directory structure
typedef struct __attribute__ ((packed)) {
  uint32_t present         :1;
  uint32_t read_write      :1;
  uint32_t user_supervisor :1;
  uint32_t write_through   :1;
  uint32_t cache_disable   :1;
  uint32_t accessed        :1;
  uint32_t dirty           :1;
  uint32_t page_size       :1;
  uint32_t global          :1;
  uint32_t available       :3;
  uint32_t page_addr       :20;
} pde_4MB_t;

// union of 4kb and 4mb pde structs
typedef union pde_t {
  pde_4KB_t pde_KB;
  pde_4MB_t pde_MB;
} pde_t;

// Page Table Entry for 4 KB Page(goes into the PD)
typedef struct __attribute__ ((packed)) {
  uint32_t present         :1;
  uint32_t read_write      :1;
  uint32_t user_supervisor :1;
  uint32_t write_through   :1;
  uint32_t cache_disable   :1;
  uint32_t accessed        :1;
  uint32_t dirty           :1;
  uint32_t pg_atrr_table   :1;
  uint32_t global          :1;
  uint32_t available       :3;
  uint32_t page_addr       :20;
} pte_4KB_t;

// create variable for terminal number
int cur_terminal;

// page directory of 1024 entries
pde_t page_directory[Num_Entries] __attribute__((aligned (PG_Size)));

// page table of 1024 entries
pte_4KB_t page_table[Num_Entries] __attribute__((aligned (PG_Size)));

// page table of 1024 entries
pte_4KB_t page_table_vidmap[Num_Entries] __attribute__((aligned (PG_Size)));

extern void process_paging(int pid);
extern void video_paging_from_addr();
extern void clean_page();
extern void mult_terminal_paging();
extern void user_video_unmap();
extern void change_uvid_putc (int terminal, int pit_terminal);
extern void schedule_paging(int pit_terminal);
extern void switch_terminal (int terminal);
extern void save_and_restore_terminal_state(int switch_to_terminal);

#endif /* PAGING_H */
