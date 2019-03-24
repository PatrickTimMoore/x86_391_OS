#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc_handler.h"
#include "keyboard_handler.h"
#include "filesystem.h"


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




//verylargetextwithverylongname.tx

// int filesys_tests(){
// 	uint8_t temp;
// 	int i;
// 	printf("starting filesystem tests:\n" );
// 	printf("making string:\n" );
// 	char file_str[] = "frame0.txt";
// 	printf("initializing host dentry\n" );
// 	dentry_t d;
// 	printf("calling read_dentry_by_name:\n" );
// 	if(!read_dentry_by_name(file_str, &d)){
// 		printf("reading by fname found\n");
// 		temp = d.file_name[31];
// 		d.file_name[31] = '\0';
// 		printf("File name: %s, File type: %d, Inode: %d----------------\n", d.file_name, d.file_type, d.inode_num);
// 		d.file_name[31] = temp;
// 	}	
// 	else{
// 		printf("Failed reading by fname\n");
// 		return FAIL;
// 	}

// 	uint8_t buf[4];
// 	buf[0] = 1;
// 	buf[1] = 2;
// 	buf[2] = 3;
// 	buf[3] = 4;
// 	if(!read_data(d.inode_num, 0, buf, 4)){
// 		printf("%s\n", "Read success! Here is what was read:" );
// 		for(i = 0; i < 4; i++){
// 			printf("0x%c,", buf[i]);
// 		}
// 		printf("%s\n", "<-----------------BYTES" );
// 	}
// 	else{
// 		printf("%s\n", "Data read unsuccessful.");
// 		return FAIL;
// 	}

// 	return 0;
// }

int filesys_tests_dir_read(){
	// clear();
	int i, byt;
	printf("Filesys dir read tests\n");
	char file_str[] = ".";
	printf("Opening .\n");
	if(open_dir((uint8_t*)file_str) == -1){
		printf("Could not open .\n");
		return FAIL;
	}
	uint8_t buf[33];

	byt = 1;
	while(byt){
		byt = read_dir(0, (void *)buf, 32);
		if(byt != -1){
			// printf("3rd Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			printf("%s\n", " ");
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_dir(0);
			return FAIL;
		}
	}

	close_dir(0);
	printf("Done printing, check screen for correctness.\n");
	return PASS;
}

int filesys_fail_cases(){
	// clear();
	// int i, byt;
	uint8_t buf[33];
	char file_str[] = "a";
	char file_str2[] = ".";
	char file_str3[] = "rtc";
	char file_str4[] = "hello";
	if(open_file((uint8_t*)file_str) != -1){
		printf("Opened nonexistent file.\n");
		return FAIL;
	}
	if(open_dir((uint8_t*)file_str) != -1){
		printf("Opened nonexistent directory.\n");
		return FAIL;
	}
	if(open_file((uint8_t*)file_str2) != -1){
		printf("Opened directory as a file.\n");
		return FAIL;
	}
	if(open_dir((uint8_t*)file_str4) != -1){
		printf("Opened file as a directory.\n");
		return FAIL;
	}
	if(open_file((uint8_t*)file_str3) != -1){
		printf("Opened rtc as a file.\n");
		return FAIL;
	}
	if(open_dir((uint8_t*)file_str3) != -1){
		printf("Opened rtc as a directory.\n");
		return FAIL;
	}

	if(read_file(0, (void*)buf, 32) != -1){
		printf("Was able to read without opening.\n");
		return FAIL;
	}
	if(open_file((uint8_t*)file_str4) == -1){
		printf("Could not open.\n");
		return FAIL;
	}
	if(read_file(0, NULL, 32) != -1){
		printf("Was able to pass NULL pointer.\n");
		return FAIL;
	}

	if(read_file(0, NULL, -32) != -1){
		printf("Was able to pass negative number of bytes to read.\n");
		return FAIL;
	}

	if(read_dir(0, (void*)buf, 32) != -1){
		printf("Was able to read without opening.\n");
		return FAIL;
	}
	if(open_dir((uint8_t*)file_str2) == -1){
		printf("Could not open.\n");
		return FAIL;
	}
	if(read_dir(0, NULL, 32) != -1){
		printf("Was able to pass NULL pointer.\n");
		return FAIL;
	}
	if(read_dir(0, NULL, -32) != -1){
		printf("Was able to pass negative number of bytes to read.\n");
		return FAIL;
	}

	close_file(0);
	close_dir(0);
	if(read_file(0, buf, 32) != -1){
		printf("Was able to read without opening.\n");
		return FAIL;
	}
	if(read_dir(0, buf, 32) != -1){
		printf("Was able to read without opening.\n");
		return FAIL;
	}

	return PASS;
}




int filesys_tests_dir_read_partial(int nbytes){
	// clear();
	int i, byt;
	printf("Filesys dir read (partial) tests (%d bytes per name)\n", nbytes);
	char file_str[] = ".";
	printf("Opening .\n");
	if(open_dir((uint8_t*)file_str) == -1){
		printf("Could not open .\n");
		return FAIL;
	}
	uint8_t buf[33];

	byt = 1;
	while(byt){
		byt = read_dir(0, (void *)buf, nbytes);
		if(byt != -1){
			// printf("3rd Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			printf("%s\n", " ");
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_dir(0);
			return FAIL;
		}
	}

	close_dir(0);
	printf("Done printing, check screen for correctness.\n");
	return PASS;
}

int filesys_file_driver_basic(){
	// clear();
	int i, byt;
	// clear();
	printf("filesys_file_driver_basic\n");
	printf("Reading from frame0.txt...\n");
	char file_str[] = "frame0.txt";
	char file_str2[] = "frame1.txt";
	if(open_file((uint8_t*)file_str) == -1){
		printf("Failed to open\n");
		return FAIL;
	}
	uint8_t buf[200];
	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("%s\n", "<-----------------BYTES" );
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}
	// read_file((void *)buf, 4);
	printf("Done reading, check screen to see what was read to verify correctness\n");
	close_file(0);


	printf("Reading from frame1.txt...\n");
	if(open_file((uint8_t*)file_str2) == -1){
		printf("Failed to open\n");
		return FAIL;
	}


	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}

	// printf("Done reading, check screen to see what was read to verify correctness\n");
	close_file(0);

	return PASS;

}



int filesys_file_driver_noclose(){
	int i, byt;
	// clear();
	printf("filesys_file_driver_noclose\n");
	printf("Reading from frame0.txt...\n");
	char file_str[] = "frame0.txt";
	char file_str2[] = "frame1.txt";
	if(open_file((uint8_t*)file_str) == -1){
		printf("Failed to open\n");
		return FAIL;
	}
	uint8_t buf[200];
	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("%s\n", "<-----------------BYTES" );
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}
	// read_file((void *)buf, 4);
	printf("Done reading, check screen to see what was read to verify correctness\n");
	// close_file(0);
	//WE DON'T CLOSE, SHOULD STILL SWITCH FINE

	printf("Reading from frame1.txt...\n");
	if(open_file((uint8_t*)file_str2) == -1){
		printf("Failed to open\n");
		return FAIL;
	}


	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("%s\n", "<-----------------BYTES" );
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}

	printf("Done reading, check screen to see what was read to verify correctness\n");
	close_file(0);

	return PASS;

}

int filesys_file_driver_exec(){
	int i, byt;
	// clear();
	printf("filesys_file_driver_noclose\n");
	printf("Reading from fish...\n");
	char file_str[] = "fish";
	// char file_str2[] = "fish";
	if(open_file((uint8_t*)file_str) == -1){
		printf("Failed to open\n");
		return FAIL;
	}
	uint8_t buf[200];
	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}
	// read_file((void *)buf, 4);
	printf("Done reading, check screen to see what was read to verify correctness\n");
	close_file(0);

	return PASS;
}


int filesys_file_driver_raw(){
	int i, byt;
	// clear();
	printf("filesys_file_driver_raw\n");
	char file_str2[] = "hello";
	uint8_t buf[200];
	printf("Reading from hello...\n");
	if(open_file((uint8_t*)file_str2) == -1){
		printf("Failed to open\n");
		return FAIL;
	}


	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("\n");
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}



	printf("\nDone reading, check screen to see what was read to verify correctness\n");
	close_file(0);

	// printf("filesys_file_driver_raw\n");
	char file_str[] = "ls";
	// uint8_t buf[200];
	printf("Reading from ls...\n");
	if(open_file((uint8_t*)file_str) == -1){
		printf("Failed to open\n");
		return FAIL;
	}


	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("\n");
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}
	printf("\nDone reading, check screen to see what was read to verify correctness\n");
	close_file(0);


	return PASS;
}


int filesys_file_driver_long(){
	int i, byt;
	// clear();
	printf("filesys_file_driver_long\n");
	char file_str2[] = "verylargetextwithverylongname.tx";
	uint8_t buf[200];
	printf("Reading from verylargetextwithverylongname.tx...\n");
	if(open_file((uint8_t*)file_str2) == -1){
		printf("Failed to open\n");
		return FAIL;
	}

	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)buf, 30);
		if(byt != -1){
			// printf("Read success (%d bytes read)! Here is what was read:\n", byt);
			for(i = 0; i < byt; i++){
				printf("%c", buf[i]);
			}
			// printf("\n");
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return FAIL;
		}
	}

	printf("\nDone reading, check screen to see what was read to verify correctness\n");
	return PASS;
}


int filesys_file_list(){
	// clear();
	int i, byt;
	dentry_t d;

	printf("Listing files...\n");
	char file_str[] = ".";
	// printf("Opening .\n");
	if(open_dir((uint8_t*)file_str) == -1){
		printf("Could not open .\n");
		return FAIL;
	}
	uint8_t buf[33];
	// uint8_t buf2print[80];

	byt = 1;
	while(byt){
		byt = read_dir(0, (void *)buf, 33);
		if(!byt){break;}
		if(byt != -1){
			// printf("File Name%s\n", buf);
			read_dentry_by_name(buf, &d);
			printf("file name: %s", buf);
			for (i = 0; i < 33 - strlen((int8_t*)buf); ++i){
				printf(" ");
			}
			printf("file type: %d, file size: %d\n", d.file_type, inodes[d.inode_num].len_b);
		}
		else{
			printf("%s\n", "Data read unsuccessful.");
			close_dir(0);
			return FAIL;
		}
	}

	close_dir(0);
	printf("Done printing, check screen for correctness.\n");
	return PASS;
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

	TEST_OUTPUT("filesys_file_driver_basic", filesys_file_driver_basic());
	// filesys_file_driver_basic();

	TEST_OUTPUT("filesys_file_driver_noclose",filesys_file_driver_noclose());
	// filesys_file_driver_noclose();

	TEST_OUTPUT("filesys_file_driver_exec",filesys_file_driver_exec());
	// filesys_file_driver_exec();

	TEST_OUTPUT("filesys_file_driver_raw",filesys_file_driver_raw());
	// filesys_file_driver_raw();

	TEST_OUTPUT("filesys_file_driver_long",filesys_file_driver_long());
	// filesys_file_driver_long();

	TEST_OUTPUT("filesys_tests_dir_read",filesys_tests_dir_read());
	// filesys_tests_dir_read();

	TEST_OUTPUT("filesys_tests_dir_read_partial(6)", filesys_tests_dir_read_partial(6));
	// filesys_tests_dir_read_partial(6);

	TEST_OUTPUT("filesys_fail_cases",filesys_fail_cases());
	
	TEST_OUTPUT("filesys_file_list",filesys_file_list());



}
