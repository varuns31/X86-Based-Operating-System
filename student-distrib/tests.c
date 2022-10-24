#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "fs.h"

#include "rtc.h"	
#include "keyboard.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure() {
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide by Zero test
 * 
 * Asserts that exception is triggered in IDT and executed properly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT divide by zero exception
 * Files: init_idt.c
 */
int idt_exception_divide_by_zero() {
	TEST_HEADER;
	int a = 1;
	int b = 0;
	int c = a/b;
	return c;
}

/* Page Fault Test
 * 
 * Asserts that page fault works - boundaries of the pages
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Pages/ Validation of pages
 * Files: paging.c/h, enable_paging.S
 */
int page_test() {
	int result = PASS;

	int *testval;

	// testval = (int *)0xb7000;
	// testval = (int *)0xb8001;
	// testval = (int *)0x3FFFFF;
	// testval = (int *)0x400001;
	// testval = (int *)0x800001;
	*testval = 5;
	return result;
}

/* Page Fault Test
 * 
 * Asserts that page fault works - boundaries of the pages
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Pages/ Validation of pages
 * Files: paging.c/h, enable_paging.S
 */
int rtc_test() {
	set_screen(0, 0);

	int i, j;
	int num = 2;

	for(i = 15; i >= 7; i--){
		rtc_handler_set_rate(i);
		for(j = 0; j <= num; j++){
			if (j % 80 == 0) set_screen(get_screen_x(), get_screen_y() + 1);
			if (! rtc_read(0, 0, 0)) putc('1');
		}
		clear();
		set_screen(0, 0);
		num *= 2;
	}


	return PASS;
}

/* Checkpoint 2 tests */
int fs_test_read_dir() {
	set_screen(0,0);
    int i = 0;
    for(i = 0; i < 10; i++){

        uint8_t* buf;
        uint8_t buff[32];
        buf = buff;

        int test_directory_read_val = directory_read(i, buf, 0);
		// not required...
		test_directory_read_val++;

        printf("File Name: ");
        int j = 32 - strlen((char*) buf);
        while(j) {
            putc(' ');
            j--;
        }

        // print name
        puts((char*) buf);
        printf(", file type: %d", our_boot_block->dir_entries[i].file_type);

        // print size
        // create_boot_block(abn_ptr);
        inode_ptr = abn_ptr + ABN_JUMP;
        inode_ptr += ABN_JUMP*(our_boot_block->dir_entries[i].inode_number);
        uint32_t inode_length = *(inode_ptr);
        printf(", file size: %d", inode_length);
        
        putc('\n');
    }

	return PASS;
}

int fs_test_fopen() {
	return PASS;
}

int fs_test_fclose() {
	return PASS;
}

int fs_test_fread() {
	const char* filename = "verylargetextwithverylongname.txt";
    int cur_fd = fs_open((char*) filename);
    set_screen(0,0);
    uint8_t* buf;
    uint8_t buff[512];
    buf = buff;
    
	printf("READING THE FILE ATTEMPT 1: \n");
	int fs_read_test_val = fs_read(cur_fd, buf, 512);
    puts((char*)buff);
    printf("\nReturn value for read: %d\n", fs_read_test_val);
	printf("DONE FILE ATTEMPT 1\n");
	
	printf("READING THE FILE ATTEMPT 2: \n");
	fs_read_test_val = fs_read(cur_fd, buf, 512);
	puts((char*)buff);
    printf("\nReturn value for read: %d\n", fs_read_test_val);
	printf("DONE FILE ATTEMPT 2\n");

	return PASS;
}

int fs_test_fwrite() {
	return PASS;
}

int terminal_test_read() {

	int fd = terminal_open("terminal");
	
	char buf[200];
	int retval = terminal_read(fd, buf, 128);
	puts(buf);
	printf("Bytes read are %d\n", retval);

	terminal_close(fd);
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() { 
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_exception_divide_by_zero", idt_exception_divide_by_zero());
	// TEST_OUTPUT("PF Test", page_test());

	// TEST_OUTPUT("Read dir test", fs_test_read_dir());
	// TEST_OUTPUT("Read file test", fs_test_fread());
	TEST_OUTPUT("Terminal read", terminal_test_read());
}
