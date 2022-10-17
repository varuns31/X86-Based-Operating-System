#ifndef _ENPAGE_H
#define _ENPAGE_H
#ifndef ASM

// define asembly linkage functions
extern void enablePaging();
extern void loadPageDirectory(pde_t* address);

#endif /* ASM */
#endif /* _ENPAGE_H */
