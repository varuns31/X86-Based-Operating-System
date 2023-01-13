# set the page directory pointer inside cr3
.text
.globl loadPageDirectory
loadPageDirectory:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
ret

# enable paging by appropriately seting cr4 and cr0
.text
.globl enablePaging
enablePaging:
    push %ebp
    mov %esp, %ebp
    mov %cr4,%eax
    # setting the bit 2 to enable paging (by the docs)
    or 0x00000010,%eax
    mov %eax, %cr4
    mov %cr0, %eax
    # setting the bit 1 to enable paging (by the docs)
    or $0x80000001, %eax
    mov %eax, %cr0
    mov %ebp, %esp
    pop %ebp
ret

.text
.globl flush_tlb
# flushing the TLB by swapping
flush_tlb:
    mov %cr3, %eax
    mov %eax, %cr3
ret
