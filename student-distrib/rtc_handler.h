
/* The header file for handling RTC*/

#ifndef _RTC_HANDLER_H
#define _RTC_HANDLER_H



#define RTC_PORT1    0x70
#define RTC_PORT2    0x71
//achievable rtc frequencies for our design
#define RTC_2Hz    	    2
#define RTC_4Hz    	    4
#define RTC_8Hz    	    8
#define RTC_16Hz      	16
#define RTC_32Hz      	32
#define RTC_64Hz        64
#define RTC_128Hz    	128
#define RTC_256Hz    	256
#define RTC_512Hz    	512
#define RTC_1024Hz      1024
//frequency values used to change the control resgister 
#define RTC_2Hz_HEX 		0x0F
#define RTC_4Hz_HEX   		0x0E
#define RTC_8Hz_HEX   		0x0D
#define RTC_16Hz_HEX  		0x0C
#define RTC_32Hz_HEX  	    0x0B
#define RTC_64Hz_HEX   	    0x0A
#define RTC_128Hz_HEX   	0x09
#define RTC_256Hz_HEX   	0x08
#define RTC_512Hz_HEX   	0x07
#define RTC_1024Hz_HEX  	0x06
/* function used to initialize the rtc*/
void init_rtc();

/*handler function for rtc*/
void rtc_handler();

/*this function open the rtc and set frequency to 2 Hz*/
int32_t rtc_open(const uint8_t * filename);

/*this function block until the next interrupt occurs*/
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);

/*this function set the RTC to desired frequency*/
int32_t  rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/*this function close the RTC*/
int32_t rtc_close(int32_t fd);

/*change the frequency of the RTC*/
void change_frequency(int32_t rate);


#endif
