#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc_handler.h"
#include "keyboard_handler.h"


#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

//CHECKPOINT 1 ALLOCATED MEMORY PAGES
#define KERNEL_ADDR_START 	0x400000
#define KERNEL_LAST_4BYTE 	0x7ffffc
#define VMEM_ADDR_START 	0xB8000
#define VMEM_LAST_4BYTE		0xB8ffc
#define KERNEL_LAST_BYTE_0	0x7ffffd
#define KERNEL_LAST_BYTE_1	0x7ffffe
#define KERNEL_LAST_BYTE_2	0x7fffff
#define VMEM_LAST_BYTE_0	0xB8ffd
#define VMEM_LAST_BYTE_1	0xB8ffe
#define VMEM_LAST_BYTE_2	0xB8fff

//CHECKPOINT 1 UNALLOCATED ADDRESSES
#define ZERO_ADDR 					0x0
#define LAST_ADDR_BEFORE_KERNEL		0x3FFFFF
#define LAST_ADDR_BEFORE_VMEM		0xB7FFF
#define FIRST_ADDR_AFTER_KERNEL		0x400001
#define FIRST_ADDR_AFTER_VMEM		0xB9000
//system call number
#define SYSTEM_CALL_VECTOR          0x80
//RTC interrupt number
#define RTC_VECTOR                  0x28
#define KEYBOARD_VECTOR             0x21
//number of exception used
#define EXCEPTION_NUM               32



static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that the 20 used exceptions, the system call and the interrupts
 * are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < EXCEPTION_NUM; ++i){

		//first check all the used exception
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	//check if the system call entry is existed
	if ((idt[SYSTEM_CALL_VECTOR].offset_15_00 == NULL) && 
			(idt[SYSTEM_CALL_VECTOR].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
	//check if the interrupts for RTC and keyboard
	if ((idt[RTC_VECTOR].offset_15_00 == NULL) && 
			(idt[RTC_VECTOR].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
	if ((idt[KEYBOARD_VECTOR].offset_15_00 == NULL) && 
			(idt[KEYBOARD_VECTOR].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
	}
 
	return result;
}


/* Exception Test 
 * 
 * Description: Test the exception handler by a division of zero.
 *              It should print out the exception message.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int idt_test_exception() {
	TEST_HEADER;
	int num1= 1;
	int num2= 0;
	int result;
	//this should trigger a exception
	result= num1 / num2;
	return FAIL; 
}


/**
 *	Test the interrupt handler by requesting keystrokes
 */
/* Keyboard Interrupt Test
 *
 * Description: Test the interrupt handler by requesting keystrokes
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
int keyboard_test_interrupt() {
	TEST_HEADER;

	printf("If all matches, this test passed :) ");
	printf("Please type the letters/numbers s3ndhe1p: ");
	// No way to automatically check for success
	return 0;
}



/**
 *	Test the paging by passing the absolute limits of what should
 	be the limits of paging addresses.
 */
/* valid_paging_tests()
 * 
 * Description: Test accessing the borders of paged present memory.
 	Should be no pagefaults. If it makes it through without any, we're good.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int valid_paging_tests(){
	//If statement contents are so that compiler doesn't optimize
	TEST_HEADER;
	int* a;
	char* b;
	int result = PASS;
	//PASS IF NO PAGEFAULTS
	//ACCESS EACH MEMORY LOCATION AND DRAG OUT 4 BYTES
	a = (int*)KERNEL_ADDR_START;
	if(!(*a));

	a = (int*)VMEM_ADDR_START;
	if(!(*a)){
		a++;
	}
	a = (int*)KERNEL_LAST_4BYTE;
	if(!(*a)){
		a++;
	}
	a = (int*)VMEM_LAST_4BYTE;
	if(!(*a)){
		a++;
	}
	
	//TRY EXTRACTING BYTES AT PAGE OVERLAP LEVEL
	b = (char*)VMEM_LAST_BYTE_0;
	if(!(*b)){
		b++;
	}

	b = (char*)VMEM_LAST_BYTE_1;
	if(!(*b)){
		b++;
	}

	b = (char*)VMEM_LAST_BYTE_2;
	if(!(*b)){
		b++;
	}
	
	b = (char*)KERNEL_LAST_BYTE_0;
	if(!(*b)){
		b++;
	}
	
	b = (char*)KERNEL_LAST_BYTE_1;
	if(!(*b)){
		b++;
	}
	
	b = (char*)KERNEL_LAST_BYTE_2;
	if(!(*b)){
		b++;
	}

	return result;
}

/**
 *	Test the paging by passing addresses that SHOULD be causing pagefaults.
 	A suggested couple are given above. PULLS 4 BYTES.
 */
/* invalid_paging_tests()
 * 
 * Description: Test accessing the invalid memory. Should suitable memory be 
   passed, should result in pagefault (anf pass/fail matters not).
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int invalid_paging_tests(int* a){
	TEST_HEADER;
	int result = FAIL;
	//Should cause pagefault if passed an invalid address
	int b = (*a);

	//JUST MAKE THE WARNING STFU
	if(b){
		return result;
	}
	else{
		return result;
	}
}

/**
 *	Test the paging by passing addresses that SHOULD be causing pagefaults.
 	A suggested couple are given above. PULLS 1 BYTE.
 */
/* invalid_paging_tests_byte()
 * 
 * Description: Test accessing the invalid memory. Should suitable memory be 
   passed, should result in pagefault (anf pass/fail matters not).
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 */
int invalid_paging_tests_byte(char* a){
	TEST_HEADER;
	int result = FAIL;
	//Should cause pagefault if passed an invalid address
	int b = (*a);

	//JUST MAKE THE WARNING STFU
	if(b){
		return result;
	}
	else{
		return result;
	}
}
// add more tests here

/* Checkpoint 2 tests */

/*
 * Description: Test if the functionality of rtc_open, 
   rtc_read and rtc_write() all work correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: we will see that the '1'
 * appears on the screen with different frequencies
 */
void rtc_driver_test(){

    //loop varibales
	int loop;
	//hold the number of 1s to write
	int count_num;
	//hold the upper bound of 1s for each frequecy
	int count_bound=4;
	//the frequency for RTC
	uint32_t freq = 2;		
	//open the rtc
	rtc_open(NULL);
	//we will check the frequency from 2 to 1024
	for (loop=0; loop<10; loop++)		
	{
		//clear the screen
        clear();
	    //initialize the counts of number of 1s
		count_num=0;
		while(count_num < count_bound)
		{
			//test the read
			rtc_read(0,0,0);		
			//print out the ‘1’ for each interrupt
			putc('1');	
			//update the count of 1s
			count_num++;	
		}
		//update the frequency
		freq= freq * 2;
		count_bound=count_bound*2;
		rtc_write(0, &freq, 4);	
		//put the cursor back to the upper left corner
		set_cursor_pos(0, 0);	
	}
}


/*
 * Description: Test if the terminal_read and terminal_write
 * work correctly.
 * Inputs: None
 * Outputs: None
 * Side Effects: we will see the texts are echoed back
 * when an enter key is hit
 */
void terminal_driver_test(){
    uint8_t buf[200];
    terminal_open(0);
    while(1){
        terminal_read (0, buf, 200);
        terminal_write (0, buf, 200);
    }
    terminal_close(0);
}



/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){


	// launch your tests here
	//checkpoint 1 test
    //TEST_OUTPUT("idt_test", idt_test());
	//idt_test_exception();
	//TEST_OUTPUT("valid_paging_tests", valid_paging_tests());
	//invalid_paging_tests(ZERO_ADDR);

	//checkpoint2 test
	rtc_driver_test();


}
