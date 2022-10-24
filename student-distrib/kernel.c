/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "fs.h"

#define RUN_TESTS

void initIdtFunc();
extern void paging_init();

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

// used to file system
uint32_t fs_mod_start;
dentry_t test_dentry;

// dentry_t test_dentry_1;

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();


    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        
        fs_mod_start = mod->mod_start;

        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* Init the IDT */
    initIdtFunc();
    
    /* Init the PIC */
    i8259_init();

    /* Init the Paging */
    paging_init();
    
    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... 
    */
    clear();
    keyboard_handler_init();
    rtc_handler_init();

    create_file_array(); 

    create_boot_block(fs_mod_start);  

    /* Testing directory read*/
    // set_screen(0,0);

    // int i = 0;
    // for(i = 0; i < 15; i++){

    //     uint8_t* buf;
    //     uint8_t buff[32];
    //     buf=buff;

    //     int test_directory_read_val = directory_read(i, buf, 0);

    //     printf("File Name: ");
    //     int j = 32 - strlen(buf);
    //     while(j){
    //         putc(' ');
    //         j--;
    //     }

    //     // print name
    //     puts(buf);
    //     printf(", file type: %d", our_boot_block->dir_entries[i].file_type);

    //     // print size
    //     // create_boot_block(abn_ptr);
    //     inode_ptr = abn_ptr + ABN_JUMP;
    //     inode_ptr += ABN_JUMP*(our_boot_block->dir_entries[i].inode_number);
    //     uint32_t inode_length = *(inode_ptr);
    //     printf(", file size: %d", inode_length);
        
    //     putc('\n');
    // }


    // /* Testing file open and read*/
    // int cur_fd = fs_open("verylargetextwithverylongname.txt");

    // set_screen(0,0);

    // uint8_t* buf;
    // uint8_t buff[512];
    // buf=buff;
    // // const unsigned int test_inode_num = 44;
    // int fs_read_test_val = fs_read(cur_fd,buf,512);
    // puts((char*)buff);
    // printf("\nReturn value for read: %d\n", fs_read_test_val);

    // /*Testing close and read*/
    // int fs_close_ret_val = fs_close(cur_fd);
    // printf("Return value for close: %d\n", fs_close_ret_val);

    // uint8_t* buf2;
    // uint8_t buf2f[512];
    // buf2=buf2f;
    // printf("Trying to read closed file:\n");
    // fs_read_test_val = fs_read(cur_fd,buf2,174);
    // printf("Return value for read: %d\n", fs_read_test_val);
    // puts((char*)buf2f);



    // int32_t test_val = read_dentry_by_name("verylargetextwithverylongname.txt", &test_dentry);
    // printf("Testing read_dentry_by_name\n");
    // puts(test_dentry.file_name);
    // printf("\n  Inode Number: %d", test_dentry.inode_number);

    // set_screen(0,0);
    // uint8_t* buf;
    // uint8_t buff[512];
    // buf=buff;
    // const unsigned int test_inode_num = 47;
    // int test_val = read_data(test_inode_num,0,buf,174);
    // puts((char*)buff);
    // printf("%d", test_val);

    // int32_t test_val = read_dentry_by_index(2, &test_dentry);
    // printf("Testing read_dentry_by_index\n");
    // puts(test_dentry.file_name);
    // printf("\n  Return value: %d", test_val);

    /*RTC TESTING*/
    // unsigned int val = 16;
    // unsigned int * test_rtc_buf = &val;

    // int test_rtc_write = rtc_write(0, test_rtc_buf, sizeof(test_rtc_buf));

    // unsigned int * test_rtc_filename = &val;
    // int test_rtc_open = rtc_open(test_rtc_filename);





    /* Enable interrupts */
    sti();
    
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    /*printf("Enabling Interrupts\n");
    sti();*/

#ifdef RUN_TESTS
    /* Run tests */
    launch_tests();
#endif
    /* Execute the first program ("shell") ... */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;"); 
}
