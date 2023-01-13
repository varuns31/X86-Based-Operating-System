#define ASM     1
#include "syscall_linkage.h"

.text
.globl syscall_linkage

# syscall assembly linkage
syscall_linkage:
	
    # compare conditon > 10
	cmpl $10, %eax
	jg invalid_call

	# compare condition < 1
	cmpl $1, %eax
	jl invalid_call

    # caller save
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    
    # jumping to jmp table
    call *jump_table(, %eax, 4)

    # caller teardown
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp 

    sti

    iret

    # return -1
    invalid_call:
        sti
        movl $-1, %eax
iret

jump_table:
.long system_dummy, system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn
