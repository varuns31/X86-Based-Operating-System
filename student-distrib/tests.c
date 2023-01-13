#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "fs.h"

#include "rtc.h"	
#include "keyboard.h"
#include "syscall_linkage.h"

extern int32_t system_execute (const uint8_t* command);
extern int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t system_close (uint32_t fd);

// taken from ece391syscalls
#define DO_CALL2(number)       \
asm volatile ("                \
    PUSHL	%EBX              ;\
	MOVL	$" #number ",%EAX ;\
	INT	$0x80             ;\
1:	POPL	%EBX              ;\
")

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

	//check the first 10 IDT entries and if they are null 
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
	//Random Value for testing
	int a = 1;
	//Random Value for testing
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
	// video memory from b8000 to b9000
	// testval = (int*) 0xb7000; 
	// testval = (int*) 0xb8001;

	// kernel memory from 4MB to 8MB
	// testval = (int*) 0x3FFFFF;  // outside
	// testval = (int*) 0x400001;  // inside
	// testval = (int*) 0x00800001; //outside 

	// Program memory from 8MB to 12MB
	// testval = (int*) 0x08048000;
	// testval = (int*) 0x08400000;

	//Random Value for testing
	testval = (int*) 0x08048018;

	//Random Value for testing
	*testval = 1;     
	return result;
}

/* Checkpoint 2 tests */

/* rtc_test
 * 
 * Asserts that read / write works for RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: may change rtc frequency
 * Coverage: Reading / writing RTC 
 * Files: rtc.c/h
 */
int rtc_test() {
	// set screen to start
	set_screen(0, 0);
	int freq, i;

	// test all frequencies from 2 to 512 hz. Multiply by 2
	for(freq = 2; freq <= 512; freq *= 2){
		// create a buffer for frequency
		int * test_rtc_buf = &freq;

		// change the rtc freq to a specific rtc freq
		rtc_write(0, test_rtc_buf, sizeof(test_rtc_buf));

		// print the characters each time an interrupt is fired. The number of chars printed will be equal to curr rtc freq
		for(i = 0; i < freq; i++){
			// there are a total of 80 chars in one line. if it we overflow start going to the next char
			if (i % 80 == 0) putc('\n');

			// once the interrupt has been fired print '1'
			if (! rtc_read(0, 0, 0)) putc('1');
		}

		// clear screen and reset its position
		clear();
		set_screen(0, 0);
	}

	return PASS;
}

/* fs_test_fopen
 * 
 * Asserts that fopen does not open an invalid file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: open for file system
 * Files: fs.c/h
 */
int fs_test_fopen() {
	// try to open invalid file
    int cur_fd = fs_open((uint8_t*) "jsi.txt");

	// reset screen to start, create a buffer, and then read the file
	set_screen(0, 0);
	uint8_t* buf;
	// random number
    uint8_t buff[512];
    buf = buff;

	//Random Value for testing
	if (fs_read(cur_fd, buf, 512) == -1){
		return FAIL;
	}

	return PASS;
}

/* fs_test_fopen
 * 
 * Asserts when we close a file, we can not read it again without opening it
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: opening fs
 * Files: fs.c/h
 */
int fs_test_fclose() {
	// open a file, reset screen, and create a buffer
    int cur_fd = fs_open((uint8_t*) "verylargetextwithverylongname.txt");
	set_screen(0, 0);
	uint8_t* buf;
	// random number
    uint8_t buff[512];
    buf = buff;

	// read and then close the file
	// random number
	fs_read(cur_fd, buf, 512);
	fs_close(cur_fd);

	// if the read does not not fail then return fail otherwise return pass
	if (fs_read(cur_fd, buf, 512) != -1) {
		return FAIL;
	}

	return PASS;
}

/* fs_test_fopen
 * 
 * Asserts that read works for fs
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: Reading / writing RTC 
 * Files: fs.c/h
 */
int fs_test_fread() {
    int cur_fd = fs_open((uint8_t*) "verylargetextwithverylongname.txt");
    set_screen(0,0);
    uint8_t* buf;
	//Random Value for testing
    uint8_t buff[512];
    buf = buff;
    
	printf("READING THE FILE ATTEMPT 1: \n");
	//Random Value for testing
	int fs_read_test_val = fs_read(cur_fd, buf, 512);
    puts((char*)buff);
    printf("\nReturn value for read: %d\n", fs_read_test_val);
	printf("DONE FILE ATTEMPT 1\n");
	
	printf("READING THE FILE ATTEMPT 2: \n");
	//Random Value for testing
	fs_read_test_val = fs_read(cur_fd, buf, 512);
	puts((char*)buff);
    printf("\nReturn value for read: %d\n", fs_read_test_val);
	printf("DONE FILE ATTEMPT 2\n");

	return PASS;
}

/* fs_test_fopen
 * 
 * Asserts write works for file system. This should return a fail as our file system is read only.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: Reading / writing RTC 
 * Files: rtc.c/h
 */
int fs_test_fwrite() {
    int cur_fd = fs_open((uint8_t*) "verylargetextwithverylongname.txt");
	set_screen(0, 0);
	uint8_t* buf;
	//Random Value for testing
    uint8_t buff[512];
    buf = buff;

	//Random Value for testing
	if (fs_write (cur_fd, buf, 512) == 1){
		return PASS;
	}

	return FAIL;
}

/* terminal_test_read
 * 
 * Asserts that open for keyboard works
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: may change rtc frequency
 * Coverage: Reading / writing keyboard.c/h
 * Files: 
 */
int terminal_test_read() {
	int fd = terminal_open((uint8_t*) "terminal");
	//Random Value for testing
	char buf[200];
	int retval;
	//Random Value for testing
	retval = terminal_read(fd, buf, 128);
	puts(buf);
	printf("Bytes read are %d\n", retval);
	terminal_close(fd);
	
	return PASS;
}

/* terminal_test_write
 * 
 * Asserts that open for terminal works
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: may change rtc frequency
 * Coverage: Reading / writing keyboard.c/h
 * Files: 
 */
int terminal_test_write() {
	int fd = terminal_open((uint8_t*) "terminal");
	//Random Value for testing
	char buf[200] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	int retval;
	//Random Value for testing
	retval = terminal_write(fd, buf, 100);
	printf("Bytes written are %d\n", retval);
	terminal_close(fd);
	
	return PASS;
}

/* terminal_test_read_write
 * 
 * Asserts that open for terminal works
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: Reading / writing keyboard.c/h
 * Files: keyboard.c/h
 */
int terminal_test_read_write() {
	int fd = terminal_open((uint8_t*) "terminal");
	//Random Value for testing
	char buf[200];
	int retval;
	//Random Value for testing
	retval = terminal_read(fd, buf, 3);
	//Random Value for testing
	retval = terminal_write(fd, buf, 3);
	terminal_close(fd);

	return PASS;
}

/* Checkpoint 3 tests */

/* open_test
 * 
 * Description: open the test function
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: sys calls
 * Files: syscall.c/h
 */
int open_test () {
	// clear screen
	set_screen(0, 0);
	
	// call open for #5 in syscall
	DO_CALL2(5);
	register int retVal asm("eax");
	printf("Return value is %d\n", retVal);

	// call open for #5 in syscall
	DO_CALL2(5);
	register int retVal2 asm("eax");
	printf("Return value is %d\n", retVal2);

	return PASS;
}

/* read_test
 * 
 * Description: read test case. open file, close file, read file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: read for fs
 * Files: fs.c/h
 */
int read_test () {
	// clear screen
	set_screen(0, 0);
	
	// call open
	DO_CALL2(5);
	register int retVal asm("eax");
	uint32_t fd = retVal;
	printf("Return value is %d\n", fd);

	// create a buffer and then read it. 1000 is the size of the buffer
	char buf[10000];
	//Random Value for testing
	system_read(fd, buf, 1000);
	puts(buf);

	// close the file
	system_close(fd);

	// then try to read it again. 1000 is the size of the buffer
	system_read(fd, buf, 1000);

	return PASS;
}

/* read_write_rtc
 * 
 * Description: try the read / write rtc function from system calls
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: may change rtc frequency
 * Coverage: 
 * Files: 
 */
int read_write_rtc () {
	// clear screen
	set_screen(0, 0);
	
	// call the open function
	DO_CALL2(5);
	register int retVal asm("eax");
	uint32_t fd = retVal;
	printf("Return value is %d\n", retVal);

	// set screen to start
	int freq, i;

	// test all frequencies from 2 to 512 hz. Multiply by 2
	for(freq = 2; freq <= 512; freq *= 2){
		// create a buffer for frequency
		int * test_rtc_buf = &freq;

		// change the rtc freq to a specific rtc freq
		system_write(fd, test_rtc_buf, sizeof(test_rtc_buf));

		// print the characters each time an interrupt is fired. The number of chars printed will be equal to curr rtc freq
		for(i = 0; i < freq; i++){
			// there are a total of 80 chars in one line. if it we overflow start going to the next char
			if (i % 80 == 0) putc('\n');

			// once the interrupt has been fired print '1'
			if (!system_read(fd, 0, 0)) putc('1');
		}

		// clear screen and reset its position
		clear();
		set_screen(0, 0);
	}

	return PASS;
}

/* Checkpoint 4 tests */

/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests() { 
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_exception_divide_by_zero", idt_exception_divide_by_zero());
	//TEST_OUTPUT("PF Test", page_test());

	// TEST_OUTPUT("Read dir test", fs_test_read_dir());
	//TEST_OUTPUT("Read file test", fs_test_fread());

	// TEST_OUTPUT("RTC test", rtc_test());
	// TEST_OUTPUT("Terminal read", terminal_test_read());
	// TEST_OUTPUT("Terminal write", terminal_test_write());
	// TEST_OUTPUT("Terminal read write", terminal_test_read_write());

	// TEST_OUTPUT("open test", open_test());
	// TEST_OUTPUT("read test", read_test());
	// TEST_OUTPUT("read test", read_write_rtc());
}

