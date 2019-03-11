/*
 ** The header file for the handler_interface.S
*/
#ifndef _HANDLER_INTERFACE_H
#define _HANDLER_INTERFACE_H

/* the interface function for keyboard */
extern void keyboard_interrupt();

/* the interface function for rtc*/
extern void rtc_interrupt();

/*handler for system call*/
extern void system_call_handler();

#endif 
