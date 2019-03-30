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
func_name:                                  ;\
  pushal                                  ;\
  pushfl                                    ;\
  call interrupt_handler                        ;\
  popfl                                   ;\
  popal                                   ;\
  iret                                    ;\

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
.global system_call_handler
system_call_handler:
  //push EBX, then ECX, then EDX
  pushl %ebp
  movl %esp, %ebp
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx

  //<insert jumptable from 1-10 using EAX>
  cmpl  $10, %eax
  jg    FAILURE
  cmpl  $1, %eax
  jl    FAILURE
  addl  $-1, %eax
  //    basically a switch function
  //    EAX holds -1 on failure or 0 on success unless specified otherwise
  call  *jump_table_system_call(, %eax, 4)
  jmp   FINISH
  FAILURE:
  xorl  %eax, %eax
  addl  $-1, %eax
  FINISH:
  //pop EDX, then ECX, then EBX, leaves EAX as return value
  popl  %ebx
  popl  %ecx
  popl  %edx
  popl  %esi
  popl  %edi
  leave
  iret

/* jump_table_system_call */
/* jump table used for the system calls */
jump_table_system_call:
  .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
