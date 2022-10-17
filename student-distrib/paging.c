
#include "paging.h"
#include "lib.h"
#include "enable_paging.h"

#define VIDEO       0xB8000                         
#define Video_memory_loc VIDEO >> 12                //Get the top 20 bits of video memory
#define kernel_mem_loc (4 * 1024 * 1024) >> 12      // kernel memory location

pde_t page_directory[Num_Entries] __attribute__((aligned (PG_Size)));
pte_4KB_t page_table[Num_Entries] __attribute__((aligned (PG_Size)));

void paging_init(){
    int i=0;
 
    for(i=0;i<Num_Entries;i++) {
        page_table[i].present=0;
        page_table[i].read_write=1;
        page_table[i].user_supervisor=0;
        page_table[i].write_through=0;
        page_table[i].cache_disable=0;
        page_table[i].accessed=0;
        page_table[i].dirty=0;
        page_table[i].pg_atrr_table=0;
        page_table[i].global=0;
        page_table[i].page_addr=i;

    }
    page_table[Video_memory_loc].present = 1;

    for(i=0;i<Num_Entries;i++) {
        page_directory[i].pde_KB.present=0;
        page_directory[i].pde_KB.read_write=1;
        page_directory[i].pde_KB.user_supervisor=0;
        page_directory[i].pde_KB.write_through=0;
        page_directory[i].pde_KB.cache_disable=0;
        page_directory[i].pde_KB.accessed=0;
        page_directory[i].pde_KB.page_size=0;
        page_directory[i].pde_KB.available=0;
        page_directory[i].pde_KB.page_addr=0x0000; 
    }

    page_directory[0].pde_KB.present = 1;
    page_directory[0].pde_KB.page_addr = (int)(page_table)>>12;
    page_directory[0].pde_KB.user_supervisor = 1;
    
    page_directory[1].pde_MB.present = 1;
    page_directory[1].pde_MB.page_size = 1;
    page_directory[1].pde_MB.page_addr = kernel_mem_loc;
    page_directory[1].pde_MB.read_write = 1;
    
    loadPageDirectory(page_directory);
    enablePaging(); 
    
}
