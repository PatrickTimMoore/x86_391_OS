/* The file to handle and intialize the keyboard and terminal */

/*
````````````````````````````````````````````````````````````````````````````````
````````000```````666``333````````PPP``RRR```OOO``BBB``L```EEE`M```M``SS````````
```````0```0`````6````````3```````P``P`R``R`O```O`B``B`L```E```MM`MM`S``````````
```````0```0`x`x`6666``333````````PPP``RRR``O```O`BBB``L```EE``M`M`M``SS````````
```````0```0``x``6```6````3```````P````R`R``O```O`B``B`L```E```M`M`M````S```````
````````000``x`x``666``333````````P````R``R``OOO``BBB``LLL`EEE`M`M`M``SS````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````But`A`Glitch`Aint`One```````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
```````````````````````````````````O.S.`3.2`````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
````````````````````````````````````````````````````````````````````````````````
*/

#include "lib.h"
#include "keyboard_handler.h"
#include "i8259.h"

#define IRQ1KEYBOARD          0x01
#define KEYBOARDDATAPORT      0x60
#define KEYBOARDCOMMANDPORT   0x64
#define SCANCODEPRESS         0x00
#define SCANCODERELEASE       0x80
#define SCANCODE_L            0x26
#define ALPHALINE1START       0x10
#define ALPHALINE1END         0x19
#define ALPHALINE2START       0x1E
#define ALPHALINE2END         0x26
#define ALPHALINE3START       0x2C
#define ALPHALINE3END         0x32
#define ESCAPEKEY             0x01
#define BACKSPACEKEY          0x0E
#define TABKEY                0x0F
#define ENTERKEY              0x1C
#define SHIFTKEY1             0x2A
#define SHIFTKEY2             0x36
#define CAPSLOCKKEY           0x3A
#define DELETEKEY             0x53
#define ALPHANUMERICVALUES    0x80
#define BACKSPACE             0x08
#define TAB                   0x09
#define ESCAPE                0x1B
#define DELETE                0x7F
#define CTRLKEY               0x1D
#define ALTKEY                0x38
#define ENTER                 '\n'
//the size of keyboard buffer
#define BUFFER_SIZE           127
#define EOS                   NULL

static int SHIFTKEYACTIVE = 0;
static int CAPSLOCKKEYACTIVE = 0;
static int CTRLACTIVE = 0;
static int ALTACTIVE = 0;
//the buffer for the keyboard
static uint8_t keyboard_buffer[BUFFER_SIZE + 1];
static int buffer_length = 0;

//flags used for terminal driver
static int read_flag = 0;
static int open_flag = 8;

/* Uppercase scancode translater */
static uint8_t scancodeUpper[ALPHANUMERICVALUES] =
    {NULL, NULL, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', NULL, NULL,              // 0x00-0x0F
     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', NULL, NULL, 'A', 'S',                // 0x10-0x1F
     'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', NULL, '|', 'Z', 'X', 'C', 'V',                 // 0x20-0x2F
     'B', 'N', 'M', '<', '>', '?', NULL, '*', NULL, ' ', NULL, NULL, NULL, NULL, NULL, NULL,          // 0x30-0x3F
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0x40-0x4F
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0x50-0x5F
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0x60-0x6F
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // 0X70-0X7F

/* Lowercase scancode translater */
static uint8_t scancodeLower[ALPHANUMERICVALUES] =
    {NULL, NULL, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', NULL, NULL,              // 0x80-0x8F
     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', NULL, NULL, 'a', 's',                // 0x90-0x9F
     'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', NULL, '\\', 'z', 'x', 'c', 'v',               // 0xA0-0xAF
     'b', 'n', 'm', ',', '.', '/', NULL, '*', NULL, ' ', NULL, NULL, NULL, NULL, NULL, NULL,          // 0xB0-0xBF
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0xC0-0xCF
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0xD0-0xDF
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 0xE0-0xEF
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // 0XF0-0XFF

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
    int isLetter = 0;
    send_eoi(IRQ1KEYBOARD);
    cli();
    scancodeVal = inb(KEYBOARDDATAPORT);
    if(((scancodeVal >= ALPHALINE1START) && (scancodeVal <= ALPHALINE1END)) ||
       ((scancodeVal >= ALPHALINE2START) && (scancodeVal <= ALPHALINE2END)) ||
       ((scancodeVal >= ALPHALINE3START) && (scancodeVal <= ALPHALINE3END))) {
        isLetter = 1;
    }
    /* Checks for special condition key presses before typing characters */
    /*    Commented out lines are placehlders for future functionality   */
    switch(scancodeVal){
        case SHIFTKEY1:
        case SHIFTKEY2:
            SHIFTKEYACTIVE = 1;
            break;
        case (SHIFTKEY1 + SCANCODERELEASE):
        case (SHIFTKEY2 + SCANCODERELEASE):
            SHIFTKEYACTIVE = 0;
            break;
        case CAPSLOCKKEY:
            CAPSLOCKKEYACTIVE ^= 1;
            break;
        case DELETEKEY:
            //putc(DELETE);
            break;
        case TABKEY:
            //putc(TAB);
            break;
        case ESCAPEKEY:
            //putc(ESCAPE);
            break;
        case CTRLKEY:
            CTRLACTIVE = 1;
            break;
        case (CTRLKEY + SCANCODERELEASE):
            CTRLACTIVE = 0;
            break;
        case ALTKEY:
            ALTACTIVE = 1;
            break;
        case (ALTKEY + SCANCODERELEASE):
            ALTACTIVE = 0;
            break;
        case BACKSPACEKEY:
            if(buffer_length != 0){
                backspace_helper();
                buffer_length--;
                keyboard_buffer[buffer_length]= NULL;
            }
            break;
        default:
        /*          Standard Character typing          */
        /*   No more than 127 characters can be typed  */
        /* 128th character is reserved for 'Enter' key */
            if(scancodeVal >= SCANCODEPRESS && scancodeVal < SCANCODERELEASE){
            	  //check if the Ctrl key is hit
                if(CTRLACTIVE){
                	  if(scancodeVal == SCANCODE_L){
                		    //clear the screen
                		    clear();
                		    //change the cursor to the upper left
                		    set_cursor_pos(0, 0);
                        break;
                	  }
                //check if the enter key is hit
                } else if(scancodeVal == ENTERKEY){
              		  read_flag = 1; //sets flag for terminal to read the keyboard_buffer
                    if(buffer_length <= BUFFER_SIZE){
                        putc(ENTER); // prints newLine character
                        keyboard_buffer[buffer_length++]=ENTER;
                    }
                //checks if it is a recognizable key
                } else if(scancodeLower[scancodeVal] == NULL){
                    //prints nothing if char value is NULL
                //checks shift and caps lock
              	} else if(SHIFTKEYACTIVE && CAPSLOCKKEYACTIVE){
                    //checks case of key that should be typed
                    if(isLetter){
                        if(buffer_length < BUFFER_SIZE){
                        	  putc(scancodeLower[scancodeVal]); // Sends data from keyboard to coonsole
                        	  keyboard_buffer[buffer_length++]=scancodeLower[scancodeVal];
                        }
                    //checks case of key that should be typed
                    } else {
                         if(buffer_length < BUFFER_SIZE){
                         	  putc(scancodeUpper[scancodeVal]); // Sends data from keyboard to coonsole
                        	  keyboard_buffer[buffer_length++]=scancodeUpper[scancodeVal];
                         }
                    }
                //checks case of key that should be typed
                } else if((CAPSLOCKKEYACTIVE && isLetter) || SHIFTKEYACTIVE){
                     if(buffer_length < BUFFER_SIZE){
                     	    putc(scancodeUpper[scancodeVal]); // Sends data from keyboard to coonsole
                        	keyboard_buffer[buffer_length++]=scancodeUpper[scancodeVal];
                     }
                } else {
                     if(buffer_length < BUFFER_SIZE){
                          putc(scancodeLower[scancodeVal]); // Sends data from keyboard to coonsole
                        	keyboard_buffer[buffer_length++]=scancodeLower[scancodeVal];
                     }
                }
            }
            //PSUDO CODE FOR POSSIBLE FUTURE USE
            //if(read_flag ==1){
                //open terminal
                //read terminal (fd, buf, nbyteslast)
            //}
    }
    /* THIS IS TEST CODE BELOW */
    //scancodeVal = inb(KEYBOARDDATAPORT);
    //printf("\n %x, %x, %c, %c \n", scancodeVal, inb(KEYBOARDCOMMANDPORT), scancodeLower[scancodeVal], scancodeUpper[scancodeVal]);
    /* END OF TEST CODE */

    sti();

}

/*following functions are for the terminal driver*/

/*
 ** int32_t terminal_open(const uint8_t * filename)
 ** Inputs: a filename which is not used
 ** Return value: always return 0
 ** Description:
 ** This function is called in the beginning to
 ** open the terminal. It set the open flag such
 ** that we can read and write
*/

int32_t terminal_open(const uint8_t * filename) {
	//check for available open flags
  if(open_flag > 0){
	   open_flag--;
     return 0;
  }
	//return 0 for success, 1 for failure
	return 1;
}

/*
 ** int32_t terminal_close(int32_t fd)
 ** Inputs: a file descripter which is not used
 ** Return value: always return 0
 ** Description:
 ** It closes the terminal driver
*/
int32_t terminal_close(int32_t fd) {
	//clear the open flag
  //Assumes terminal_open was successful
	open_flag++;
	return 0;
}

/*
 ** int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 ** Inputs: fd -- file descriptor, buf -- buffer used to read from
 ** nbytes: number of bytes to be read
 ** Return value: the number of bytes read correctly
 ** Description:
 ** This function read from the keyboard buffer a line that
 ** ends with an 'enter' key. It will wait until the last
 ** interrupt finishes brfore reading.
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    //enable the interrupt
    sti();
    //check if the terminal driver is open or not
    if(open_flag == 8){
    	return 0;
    }
	//check if the parameters are valid
	if(buf == NULL || nbytes == 0){
		return 0;
	}
	//spin to wait the last interrupt ends
	while(!read_flag);
	//clear the read flag
	read_flag = 0;
	//cast the buf into new type
	uint8_t* buf_uint = (uint8_t*)buf;
	//loop variabe
	int loop;

	for(loop = 0; loop < nbytes; loop++){

		  //check if it exceeds the the buffer size
		  if(loop > BUFFER_SIZE){
			   break;
		  }
		  //check if we hit the EOS
		  if(keyboard_buffer[loop] == NULL || (keyboard_buffer[loop - 1] == ENTER && loop > 0)){
			   break;
		  }
		  //copy into the new buffer
		  buf_uint[loop]= keyboard_buffer[loop];
	}
      //mark the end as the EOS
      buf_uint[loop] = EOS;

      //clear the buffer
      int i;
      int bytes_read = loop;
      int nullSeen = 0;
      int overflowedChar = 0;
      for(i=0; i <= BUFFER_SIZE; i++){
         //Pushes left over characters in buffer after newLine
         // to the front of the buffer. Clears the rest.
         if(nullSeen){
            keyboard_buffer[i] = NULL;
         } else {
            if(keyboard_buffer[loop] == NULL || loop >= BUFFER_SIZE){
                nullSeen = 1;
            } else {
                overflowedChar++;
            }
            keyboard_buffer[i]=keyboard_buffer[loop++];
         }
      }
      buffer_length=overflowedChar;

      //return the number of bytes read correctly
      return bytes_read;
}


/*
 ** int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 ** Inputs: fd -- file descriptor, buf -- buffer used to write to the screen
 ** nbytes: number of bytes to write to screen
 ** Return value: the number of bytes write correctly
 ** Description:
 ** This function write nbytes bytes from the keyboard buffer to the screen.
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
     //check if the terminal driver is open or not
    if(open_flag == 0){
    	return 0;
    }
	//check if the parameters are valid
	if(buf == NULL || nbytes == 0){
		return 0;
    }
    //cast the buf into new type
    uint8_t* buf_uint= (uint8_t*)buf;
	//loop variabe
	int loop;
	int bytes_written = 0;
	for(loop = 0; loop < nbytes; loop++){
		if(buf_uint[loop] == NULL){
			break;
		}
		putc(buf_uint[loop]);
		bytes_written++;
	}
  loop=0;
  while(keyboard_buffer[loop] != NULL){
      if(keyboard_buffer[loop] == '\n'){
          putc(keyboard_buffer[loop++]);
          uint8_t bufNew[200];
          terminal_open(0);
          terminal_read(0, bufNew, 200);
          bytes_written += terminal_write(0, bufNew, 200);
          terminal_close(0);
          break;
      }
      putc(keyboard_buffer[loop++]);
  }
	return bytes_written;
}
