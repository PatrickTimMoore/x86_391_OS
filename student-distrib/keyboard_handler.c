/* The file to handle and intialize the keyboard */

#include "lib.h"
#include "keyboard_handler.h"
#include "i8259.h"
#include "rtc_handler.h"

#define IRQ1KEYBOARD 0x01
#define KEYBOARDDATAPORT 0x60
#define KEYBOARDCOMMANDPORT 0x64
#define SCANCODEPRESS 0x00
#define SCANCODERELEASE 0x80
#define LOWERALPHANUMVALS 60

static uint8_t scancode[LOWERALPHANUMVALS] = {'0', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '0', '0',
     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '0', '0', 'a', 's',
     'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '0', '\\', 'z', 'x', 'c', 'v',
     'b', 'n', 'm',',', '.', '/', '0', '*', '0', ' ', '0'};

/*
 ** int init_keyboard()
 ** Inputs: none
 ** Return value: none
 ** Description:
 ** This function is called in the beginning to
 ** initialize keyboard interupts.
*/
void init_keyboard(){
    // PIC 0x21 == IRQ1 == keyboard
    enable_irq(IRQ1KEYBOARD); // Enables Keyboard interupts
}

/*
 ** int keyboard_handler()
 ** Inputs: none
 ** Return value: none
 ** Description:
 ** This function is called in the beginning to
 ** initialize keyboard interupts.
*/
void keyboard_handler(){
    // Keyboard registers are as follows
      // 0x60 - data byte // 0x64 - command byte
    unsigned int scancodeVal;
    send_eoi(IRQ1KEYBOARD);
    cli();
    if((scancodeVal = inb(KEYBOARDDATAPORT)) >= SCANCODEPRESS && scancodeVal < SCANCODERELEASE){
        putc(scancode[scancodeVal]); // Sends data from keyboard to coonsole
    }
    sti();

}
