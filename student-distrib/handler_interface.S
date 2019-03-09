/*This file sets up the interface for interrupts and system calls*/
#define ASM    1
#include "x86_desc.h"

/* interrupt handler common interface
** This is a common interface for all of the interrupt: RTC and keyboard
** In this common codes, we will save all the registers and call the specifc
** handler. After that , we will tear down the stack and call iret
** Inputs   : none
** Outputs  : none
** Registers: all the registers and flags are saved
*/

#define INTERRUPT_HANDLER(func_name, interrupt_handler)        \
.GLOBAL  func_name                                            ;\
func_name:										              ;\
	pushal  									              ;\
	pushfl									                  ;\
	call interrupt_handler							          ;\
	popfl									                  ;\
	popal									                  ;\
	iret									                  ;\

 /*the interface for keyboard handler*/
 INTERRUPT_HANDLER(keyboard_interrupt, keyboard_handler);
 /*the interface for rtc handler*/
 INTERRUPT_HANDLER(rtc_interrupt, rtc_handler);


/* system call common interface
** This is a common interface for all of the system calls
** In this common codes, we will save all the registers and call the specifc
** handler. After that , we will tear down the stack and call iret
** Inputs   : none
** Outputs  : none
** Registers: all the registers and flags are saved
*/


