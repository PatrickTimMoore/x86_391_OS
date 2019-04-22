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
**
** PUSHING EAX TWICE AS A DUMMY VARIABLE
*/

#define INTERRUPT_HANDLER(func_name, interrupt_handler)        \
.GLOBAL  func_name                                            ;\
func_name:                                ;\
  pushl %eax                              ;\
  pushl %fs                               ;\
  pushl %es                               ;\
  pushl %ds                               ;\
  pushl %eax                              ;\
  pushl %ebp                              ;\
  pushl %edi                              ;\
  pushl %esi                              ;\
  pushl %edx                              ;\
  pushl %ecx                              ;\
  pushl %ebx                              ;\
  call  interrupt_handler                 ;\
  call  sig_pending                       ;\
  popl  %ebx                              ;\
  popl  %ecx                              ;\
  popl  %edx                              ;\
  popl  %esi                              ;\
  popl  %edi                              ;\
  popl  %ebp                              ;\
  popl  %eax                              ;\
  popl  %ds                               ;\
  popl  %es                               ;\
  popl  %fs                               ;\
  addl  $4, %esp                          ;\
  iret                                    ;\

 /*the interface for keyboard handler*/
 INTERRUPT_HANDLER(keyboard_interrupt, keyboard_handler);
 /*the interface for rtc handler*/
 INTERRUPT_HANDLER(rtc_interrupt, rtc_handler);
 /*the interface for pit handler*/
 INTERRUPT_HANDLER(pit_interrupt, pit_handler);


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
  # pushfl
  pushl %fs
  pushl %es
  pushl %ds
  pushl %ebp
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
  movw  $USER_DS, %dx
  movw  %dx, %ds
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
  popl  %ebp
  popl  %ds
  popl  %es
  popl  %fs
  # popfl
  iret

/* jump_table_system_call */
/* jump table used for the system calls */
jump_table_system_call:
  .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn


  /* exception_handler common interface
** This is a common interface for all of the interrupt: RTC and keyboard
** In this common codes, we will save all the registers and call the specifc
** handler. After that , we will tear down the stack and call iret
** Inputs   : none
** Outputs  : none
** Registers: all the registers and flags are saved
**
** PUSH EAX TWICE, ONCE AS A DUMMY VARIABLE
*/

#define EXCEPTION_HANDLER(func_name, exception_handler)        \
.GLOBAL  func_name                                            ;\
func_name:                                ;\
  pushl %eax                              ;\
  pushl %fs                               ;\
  pushl %es                               ;\
  pushl %ds                               ;\
  pushl %eax                              ;\
  pushl %ebp                              ;\
  pushl %edi                              ;\
  pushl %esi                              ;\
  pushl %edx                              ;\
  pushl %ecx                              ;\
  pushl %ebx                              ;\
  call exception_handler                  ;\
  call sig_pending                        ;\
  popl  %ebx                              ;\
  popl  %ecx                              ;\
  popl  %edx                              ;\
  popl  %esi                              ;\
  popl  %edi                              ;\
  popl  %ebp                              ;\
  popl  %eax                              ;\
  popl  %ds                               ;\
  popl  %es                               ;\
  popl  %fs                               ;\
  addl  $4, %esp                          ;\
  iret                                    ;\


#define EXCEPTION_HANDLER_ERROR(func_name, exception_handler)        \
.GLOBAL  func_name                                            ;\
func_name:                                  ;\
  pushl %fs                               ;\
  pushl %es                               ;\
  pushl %ds                               ;\
  pushl %eax                              ;\
  pushl %ebp                              ;\
  pushl %edi                              ;\
  pushl %esi                              ;\
  pushl %edx                              ;\
  pushl %ecx                              ;\
  pushl %ebx                              ;\
  call exception_handler                  ;\
  call sig_pending                        ;\
  popl  %ebx                              ;\
  popl  %ecx                              ;\
  popl  %edx                              ;\
  popl  %esi                              ;\
  popl  %edi                              ;\
  popl  %ebp                              ;\
  popl  %eax                              ;\
  popl  %ds                               ;\
  popl  %es                               ;\
  popl  %fs                               ;\
  addl  $4, %esp                          ;\
  iret                                    ;\


EXCEPTION_HANDLER(exception_jump_man_0, exception_handler_0);
EXCEPTION_HANDLER(exception_jump_man_1, exception_handler_1);
EXCEPTION_HANDLER(exception_jump_man_2, exception_handler_2);
EXCEPTION_HANDLER(exception_jump_man_3, exception_handler_3);
EXCEPTION_HANDLER(exception_jump_man_4, exception_handler_4);
EXCEPTION_HANDLER(exception_jump_man_5, exception_handler_5);
EXCEPTION_HANDLER(exception_jump_man_6, exception_handler_6);
EXCEPTION_HANDLER(exception_jump_man_7, exception_handler_7);
EXCEPTION_HANDLER_ERROR(exception_jump_man_8, exception_handler_8);
EXCEPTION_HANDLER(exception_jump_man_9, exception_handler_9);
EXCEPTION_HANDLER_ERROR(exception_jump_man_10, exception_handler_10);
EXCEPTION_HANDLER_ERROR(exception_jump_man_11, exception_handler_11);
EXCEPTION_HANDLER_ERROR(exception_jump_man_12, exception_handler_12);
EXCEPTION_HANDLER_ERROR(exception_jump_man_13, exception_handler_13);
EXCEPTION_HANDLER_ERROR(exception_jump_man_14, exception_handler_14);
EXCEPTION_HANDLER(exception_jump_man_15, exception_handler_15);
EXCEPTION_HANDLER(exception_jump_man_16, exception_handler_16);
EXCEPTION_HANDLER_ERROR(exception_jump_man_17, exception_handler_17);
EXCEPTION_HANDLER(exception_jump_man_18, exception_handler_18);
EXCEPTION_HANDLER(exception_jump_man_19, exception_handler_19);
EXCEPTION_HANDLER(exception_jump_man_20, exception_handler_20);
EXCEPTION_HANDLER(exception_jump_man_21, exception_handler_21);
EXCEPTION_HANDLER(exception_jump_man_22, exception_handler_22);
EXCEPTION_HANDLER(exception_jump_man_23, exception_handler_23);
EXCEPTION_HANDLER(exception_jump_man_24, exception_handler_24);
EXCEPTION_HANDLER(exception_jump_man_25, exception_handler_25);
EXCEPTION_HANDLER(exception_jump_man_26, exception_handler_26);
EXCEPTION_HANDLER(exception_jump_man_27, exception_handler_27);
EXCEPTION_HANDLER(exception_jump_man_28, exception_handler_28);
EXCEPTION_HANDLER(exception_jump_man_29, exception_handler_29);
EXCEPTION_HANDLER(exception_jump_man_30, exception_handler_30);
EXCEPTION_HANDLER(exception_jump_man_31, exception_handler_31);
