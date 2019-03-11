
/* The header file for handling RTC*/

#ifndef _RTC_HANDLER_H
#define _RTC_HANDLER_H

#define RTC_PORT1    0x70
#define RTC_PORT2    0x71
/* function used to initialize the rtc*/
void init_rtc();

/*handler function for rtc*/
void rtc_handler();


#endif
