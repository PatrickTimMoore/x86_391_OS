/*
 ** The header file for the handler_interface.S
*/
#ifndef _HANDLER_INTERFACE_H
#define _HANDLER_INTERFACE_H

/* the interface function for keyboard */
extern void keyboard_interrupt();

/* the interface function for rtc*/
extern void rtc_interrupt();

/* the interface function for pit */
extern void pit_interrupt();

/*handler for system call*/
extern void system_call_handler();

/*handler for exceptions*/
extern void exception_jump_man_0();
extern void exception_jump_man_1();
extern void exception_jump_man_2();
extern void exception_jump_man_3();
extern void exception_jump_man_4();
extern void exception_jump_man_5();
extern void exception_jump_man_6();
extern void exception_jump_man_7();
extern void exception_jump_man_8();
extern void exception_jump_man_9();
extern void exception_jump_man_10();
extern void exception_jump_man_11();
extern void exception_jump_man_12();
extern void exception_jump_man_13();
extern void exception_jump_man_14();
extern void exception_jump_man_15();
extern void exception_jump_man_16();
extern void exception_jump_man_17();
extern void exception_jump_man_18();
extern void exception_jump_man_19();
extern void exception_jump_man_20();
extern void exception_jump_man_21();
extern void exception_jump_man_22(); 
extern void exception_jump_man_23();
extern void exception_jump_man_24();
extern void exception_jump_man_25();
extern void exception_jump_man_26();
extern void exception_jump_man_27();
extern void exception_jump_man_28();
extern void exception_jump_man_29();
extern void exception_jump_man_30();
extern void exception_jump_man_31();

#endif 
