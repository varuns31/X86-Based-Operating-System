#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

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

	//testval = (int *)0xb7000;
	//testval = (int *)0xb8001;
	///testval = (int *)0x3FFFFF;
	//testval = (int *)0x400001;
	// testval = (int *)0x800001;
	*testval = 5;
	return result;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_exception_divide_by_zero", idt_exception_divide_by_zero());
	TEST_OUTPUT("PF Test", page_test());
}
