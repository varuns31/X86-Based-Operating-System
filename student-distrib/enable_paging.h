#ifndef _ENPAGE_H
#define _ENPAGE_H
#ifndef ASM

extern void enablePaging();
extern void loadPageDirectory(pde_t* address);

#endif /* ASM */
#endif /* _ENPAGE_H */
