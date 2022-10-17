#include "x86_desc.h"
#include "lib.h"

#ifndef PAGING_H
#define PAGING_H

#define PG_Size     4096 // page table is 4Kb
#define Num_Entries 1024 // page table and page directory has 1024 entries

// has all the relevant fields for a 4kb page directory structure
typedef union pde_4KB_t {
  struct {
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
  } __attribute__ ((packed));
} pde_4KB_t;

// has all the relevant fields for a 4mb page directory structure
typedef struct pde_4MB_t {
  union {
  struct {
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
  }  __attribute__ ((packed));
  };
} pde_4MB_t;

// union of 4kb and 4mb pde structs
typedef union pde_t {
  pde_4KB_t pde_KB;
  pde_4MB_t pde_MB;
} pde_t;

// Page Table Entry for 4 KB Page(goes into the PD)
typedef struct pte_4KB_t {
  union {
      uint32_t val;
      struct {
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
      } __attribute__ ((packed));
  };
} pte_4KB_t;

#endif /* PAGING_H */
