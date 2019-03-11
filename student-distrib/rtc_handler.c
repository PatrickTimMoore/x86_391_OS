/* The file to handle and intialize the RTC */

#include "lib.h"
#include "rtc_handler.h"
#include "i8259.h"
//The index for control register A and B
#define CTRA        0x0A
#define CTRB        0x0B 
#define CTRC        0x0C
#define BIT6_MASK   0x40
#define RTC_LINE    8
/* 
 ** int init_rtc()
 ** Inputs: none
 ** Return value: none
 ** Description: 
 ** This function is called in the beginning to 
 ** initialize the rtc.
*/
void init_rtc(){
     uint32_t old_value;
     //read the old value in control register B
     outb(CTRB, RTC_PORT1);
     old_value= inb(RTC_PORT2);
     //set the bit 6 to 1 to allow the interrupts
     old_value= old_value | BIT6_MASK;
     //wrtie the updated value back to control register B
     outb(CTRB, RTC_PORT1);
     outb(old_value, RTC_PORT2);
   
     //enable the irq line for RTC
     enable_irq(RTC_LINE);
}

/* 
 ** void rtc_handler()
 ** Inputs: none
 ** Return value: none
 ** Description: 
 ** This function is used to handle the interrupt from 
 ** rtc. For the checkpoint 1, it just simply calls the
 ** 
*/
void rtc_handler(){
    send_eoi(RTC_LINE);
	//clear the interrupts
	cli();
	//we will send eoi signal before the handler

    test_interrupts();
    //input from the control register to allow another interrupt
    outb(CTRC, RTC_PORT1);
    inb(RTC_PORT2);

    sti();

}
