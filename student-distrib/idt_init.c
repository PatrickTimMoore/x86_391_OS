/*
  This file is used to initialize the each entry
  in the interrupt descriptor table.
*/

#include "idt_init.h"
#include "x86_desc.h"
#include "exception_handler.h"
#include "handler_interface.h"

#define SYSTEM_CALL_NUM  0x80
#define KEYBOARD_NUM     0x21
#define RTC_NUM          0x28
#define EXCEPTION_NUM    32


//list of exception handler address used to handle exceptions
uint32_t exception_list[EXCEPTION_NUM] = {
	(uint32_t)exception_handler_0,
	(uint32_t)exception_handler_1,
	(uint32_t)exception_handler_2,
  (uint32_t)exception_handler_3,
	(uint32_t)exception_handler_4,
	(uint32_t)exception_handler_5,
	(uint32_t)exception_handler_6,
	(uint32_t)exception_handler_7,
	(uint32_t)exception_handler_8,
	(uint32_t)exception_handler_9,
	(uint32_t)exception_handler_10,
	(uint32_t)exception_handler_11,
	(uint32_t)exception_handler_12,
	(uint32_t)exception_handler_13,
	(uint32_t)exception_handler_14,
	(uint32_t)exception_handler_15,
	(uint32_t)exception_handler_16,
	(uint32_t)exception_handler_17,
	(uint32_t)exception_handler_18,
	(uint32_t)exception_handler_19,
  (uint32_t)exception_handler_20,
  (uint32_t)exception_handler_21,
  (uint32_t)exception_handler_22,
  (uint32_t)exception_handler_23,
  (uint32_t)exception_handler_24,
  (uint32_t)exception_handler_25,
  (uint32_t)exception_handler_26,
  (uint32_t)exception_handler_27,
  (uint32_t)exception_handler_28,
  (uint32_t)exception_handler_29,
  (uint32_t)exception_handler_30,
  (uint32_t)exception_handler_31,
};

/* Function: int init_idt(void)
*  Description: this function is used to initialize each 
                entry in the idt. 
*  inputs: none
*  ouputs: 0 for success and -1 for failure
*  effects: initialize each entry in idt
*/
int init_idt(){
    //loop varibale
	int i;
	//loop through the idt to initialize each entry
	for(i=0; i< NUM_VEC; i++ ){
		  //these bits are the same for exception, interrupt and 
		  //system call
		  idt[i].present = 1;
  		idt[i].size = 1;
  		//those reserved bits are not changed
		  idt[i].reserved0 = 0;
  		idt[i].reserved1 = 1;
  		idt[i].reserved2 = 1;
  		//we choose the interrupt gate
  		idt[i].reserved3 = 0;
  		idt[i].reserved4 = 0;
  		idt[i].seg_selector = KERNEL_CS;
  		//check if it is valid exception
  		if(i < EXCEPTION_NUM){
  			//set the offset for exception
            SET_IDT_ENTRY(idt[i], exception_list[i]);
  		}
      //check if it is a system call
      else if(i == SYSTEM_CALL_NUM){
            //set the priority to user level
            idt[i].dpl = 3;
            //set the offset for system call handler
            SET_IDT_ENTRY(idt[i], system_call_handler);
      }
      else if(i == KEYBOARD_NUM ){
        	//set the offset for keyboard interrupt
        	SET_IDT_ENTRY(idt[i], keyboard_interrupt);
      }
      else if(i == RTC_NUM){
        	//set the offset for the RTC interrupt
        	SET_IDT_ENTRY(idt[i], rtc_interrupt);
      }
      else{
        	//the entries are not used yet
        	idt[i].present = 0;
      }


	}

    return 0;
}
