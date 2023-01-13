#ifndef _ENPAGE_H
#define _ENPAGE_H
#ifndef ASM

// define asembly linkage functions

/* enables paging for kernel and video memory */
extern void enablePaging();

/* flushes the TLB */
extern void flush_tlb();

/* creates and enables page at the address */
extern void loadPageDirectory(pde_t* address);

#endif /* ASM */
#endif /* _ENPAGE_H */

