
#include "paging.h"
#include "lib.h"

pde_t page_directory[Num_Entries] __attribute__((aligned (PG_Size)));
pte_4KB_t page_table[Num_Entries] __attribute__((aligned (PG_Size)));
pte_4KB_t page_table_usermap[Num_Entries] __attribute__((aligned (PG_Size)));

void paging_init(){
    int i=0;
    for(i=0;i<Num_Entries;i++)
    {
        page_table[i].present=0;
        page_table[i].addr=0x0000;
    }
    page_table[Video_memory_loc].present=1;
    page_table[Video_memory_loc].addr=
}