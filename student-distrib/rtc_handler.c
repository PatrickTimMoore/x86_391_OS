/* The file to handle and intialize the RTC */

#include "lib.h"
#include "rtc_handler.h"
#include "i8259.h"
#include "signal.h"
#include "scheduling.h"

//The index for control register A and B
#define CTRA              0x0A
#define CTRB              0x0B 
#define CTRC              0x0C
//the mask used to turn on the periodic interrupt
#define BIT6_MASK         0x40
//the mask used to set the frequency of rtc
#define FREQUENCY_MASK    0xF0
#define RTC_LINE          8

#define NUM_TERMS         3
// #define BIG_FREQ          1024
#define BIG_FREQ          512
//store the state of the RTC interrupt
volatile int rtc_interrupt_happened[NUM_TERMS];
//the flag to indicate the RTC is open
static int rtc_open_flag = 0;
//the lookup table for the requested rtc
static volatile int rtc_req[NUM_TERMS];
//tick count
static volatile uint32_t tick_count[NUM_TERMS];
//Alarm signal counter
static int alarm = 0;

/* 
 ** int init_rtc()
 ** Inputs: none
 ** Return value: none
 ** Description: 
 ** This function is called in the beginning to 
 ** initialize the rtc.
*/
void init_rtc(){
     int i;
     uint32_t old_value;
     //read the old value in control register B
     outb(CTRB, RTC_PORT1);
     old_value= inb(RTC_PORT2);
     //set the bit 6 to 1 to allow the interrupts
     old_value= old_value | BIT6_MASK;
     //wrtie the updated value back to control register B
     outb(CTRB, RTC_PORT1);
     outb(old_value, RTC_PORT2);
   
     //RUN THAT SHIT
     // change_frequency(RTC_1024Hz);

     //Maximum RTC actually runs at is 512, so we fix it to do so so our RTC-dependent programs tick consistently
     change_frequency(RTC_512Hz);

     //Initializing the bitmap
     for (i = 0; i < NUM_TERMS; ++i){
        rtc_req[i] = 1;
        tick_count[i] = 1;
        rtc_interrupt_happened[i] = 0;
     }

     //set the count
     // tick_count = 1;

     alarm = 0;

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
    //we will send eoi signal before the handler
    send_eoi(RTC_LINE);
	//clear the interrupts
	cli();
    //mark that the interrupt occured
    //UP THAT COUNT YEET
    // tick_count[run_term]++;
    tick_count[0]++;
    tick_count[1]++;
    tick_count[2]++;
    if((tick_count[0] % rtc_req[0]) == 0){
        rtc_interrupt_happened[0] = 1;
        tick_count[0] = 1;
    }
    if((tick_count[1] % rtc_req[1]) == 0){
        rtc_interrupt_happened[1] = 1;
        tick_count[1] = 1;
    }
    if((tick_count[2] % rtc_req[2]) == 0){
        rtc_interrupt_happened[2] = 1;
        tick_count[2] = 1;
    }

    alarm++;
    // printf("alarm: %d\n", alarm);
    if(alarm%5120 == 0){
        alarm = 0;
        raise_sig(ALARM);
    }

    //input from the control register to allow another interrupt
    outb(CTRC, RTC_PORT1);
    inb(RTC_PORT2);
    //enable the interrupt
 
}


/* 
 ** void rtc_open()
 ** Inputs: the pointer to the file(it is neglected )
 ** Return value: 0
 ** Description: 
 ** This function initialize the RTC to the default frequency
 ** 2Hz and always return 0. Notice that the file pointer is 
 ** not used.
*/
int32_t rtc_open(const uint8_t * filename){
    //mark the flag to indicate the rtc is open
    rtc_open_flag = 1;
    //change the frequency to defualt value: 2
    // int32_t default_freq= 2;
    // change_frequency(default_freq);
    return 0;
}


 /*
 *  int32_t rtc_read()
 *  inputs: the standard read function: file descriptor, buffer, and size of buffer
 *  return: always return 0 for success and -1 for failure
 *  Description: 
 ** This function will block until the next interrupt of RTC happens.
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){
    sti();
    //check if the RTC is open or not
    //if( rtc_open_flag == 0){
    //    return -1;
    //}
    //wait until the next interrupt happens
    // while (!rtc_interrupt_happened);    
    while(!rtc_interrupt_happened[run_term]);
    // tick_count[run_term] = 1;
    // clear the flag for rtc interrupts 
    rtc_interrupt_happened[run_term] = 0;

    //always return 0
    return 0;
 }


 /*
 *  int32_t rtc_write()
 *  inputs: the standard write function: file descriptor, buffer, and size of buffer
 *  return: the number of bytes we used to set the RTC frequency or -1 for faliure
 *  Description: 
 ** This function tries to set the RTC to desired frequency
 */
int32_t  rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    cli();
    //check if the RTC is open or not
    if( rtc_open_flag == 0){
        sti();
        return -1;
    }

    //Set the run term's thing
    int32_t ret_val;
    rtc_req[run_term] = BIG_FREQ/(*(int32_t*)buf);
    ret_val = nbytes;
    sti();
    //save the flags and disable the interrupts
    // int32_t flags;   
    // cli_and_save(flags);
    // //check if the buffer is null or we don't have four byte to write 
    // if(nbytes==4 && buf !=NULL){
    //     //set the RTC to the desired frequency
    //     change_frequency(*((int32_t*)buf));
    //     ret_val=nbytes;
    // }
    // else{
    //     //return -1 for faliure
    //     ret_val=-1;
    // }
    // //restore the flags for RTC
    // restore_flags(flags);
    return ret_val;   
}

 /*
 *  int32_t rtc_close()
 *  inputs: the standard close function: file id 
 *  return: always 0
 *  Description: 
 ** This function does nothing
 */
int32_t rtc_close(int32_t fd){
    //clear the rtc open flag
    //rtc_open_flag = 0;
    //always return 0
    return 0;
 }

/* 
 ** void change_frequency(int32_t rate)
 ** Inputs: the rate to set
 ** Return value: none
 ** Description: 
 ** This function tries to set the RTC to the 
 ** desired frequency. Notice that it only accepts
 ** frequency that is lower that 1024 in our design
 ** 
*/
void change_frequency(int32_t rate) {
    uint8_t freq=0;
    uint32_t old_value;

    //if the frequecy is out of the bound
    if(rate >1024 || rate <=0){
        return;
    }
    //check which frequency it is
    switch(rate){
        case RTC_2Hz:
            freq = RTC_2Hz_HEX ;
            break;
        case RTC_4Hz:
            freq = RTC_4Hz_HEX ;
            break;
        case RTC_8Hz:
            freq = RTC_8Hz_HEX ;
            break;
        case RTC_16Hz:
            freq = RTC_16Hz_HEX ;
            break;
        case RTC_32Hz:
            freq = RTC_32Hz_HEX ;
            break;
        case RTC_64Hz:
            freq = RTC_64Hz_HEX ;
            break;
        case RTC_128Hz:
            freq = RTC_128Hz_HEX ;
            break;
        case RTC_256Hz:
            freq = RTC_256Hz_HEX ;
            break;
        case RTC_512Hz:
            freq = RTC_512Hz_HEX ;
            break;
        case RTC_1024Hz:
            freq = RTC_1024Hz_HEX ;
            break;
        default:
            return;
    }
     
     //restore the old values for control register A
     outb(CTRA, RTC_PORT1);
     old_value= inb(RTC_PORT2);
     //set the to the desired frequency
     old_value= old_value & FREQUENCY_MASK;
     old_value=old_value | freq;
     //wrtie the updated value back to control register B
     outb(CTRA, RTC_PORT1);
     outb(old_value, RTC_PORT2);

    return;
}

