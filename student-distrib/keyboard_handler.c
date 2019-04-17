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
#include "paging.h"
#include "system_call.h"
#include "scheduling.h"

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
#define F1                    0x3B
#define F2                    0x3C
#define F3                    0x3D

//the size of keyboard buffer
#define BUFFER_SIZE           127
#define EOS                   NULL
#define FORTY_MB              0x2800000
#define FOUR_KB               4096     
#define PD_ATTRIB             0x7
#define EMPTY_PT_ENTRY        0x00000002
#define VIDEO                 0xB8000
#define NUM_COLS              80
#define NUM_ROWS              25
#define ATTRIB                0x7
#define _32_MB                0x2000000
#define _132_MB               0x8400000
#define ADDR_BLACKOUT         0xFFFFF000


static int SHIFTKEYACTIVE = 0;
static int CAPSLOCKKEYACTIVE = 0;
static int CTRLACTIVE = 0;
static int ALTACTIVE = 0;
//the buffer for the keyboard
//static uint8_t keyboard_buffer[BUFFER_SIZE + 1];
// static uint32_t term_num = 0;
//static int terms[term_num].buffer_index = 0;

//flags used for terminal driver
// static int terms[term_num].entered = 0;
static int open_flag = 8;



uint32_t fakemem_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));
uint32_t vmem_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

// terminal_t terms[NUM_TERM];

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
    // sti();
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
            if(terms[term_num].buffer_index != 0){
                backspace_helper();
                terms[term_num].buffer_index--;
                (terms[term_num].keyboard_buffer)[terms[term_num].buffer_index]= NULL;
            }
            break;
        default:
        /*          Standard Character typing          */
        /*   No more than 127 characters can be typed  */
        /* 128th character is reserved for 'Enter' key */
            if(scancodeVal >= SCANCODEPRESS && scancodeVal < SCANCODERELEASE){
                //check if the alt key is pressed
                if(ALTACTIVE){
                  if(scancodeVal == F1){
                    switch_terminal(0);
                    // printf("F1\n");
                  }
                  else if(scancodeVal == F2){
                    switch_terminal(1);
                    // printf("F2\n");
                  }
                  else if(scancodeVal == F3){
                    switch_terminal(2);
                    // printf("F3\n");
                  }
                    
                }
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
              		  terms[term_num].entered = 1; //sets flag for terminal to read the (terms[term_num].keyboard_buffer)
                    if(terms[term_num].buffer_index <= BUFFER_SIZE){
                        putc(ENTER); // prints newLine character
                        (terms[term_num].keyboard_buffer)[terms[term_num].buffer_index++]=ENTER;
                    }
                //checks if it is a recognizable key
                } else if(scancodeLower[scancodeVal] == NULL){
                    //prints nothing if char value is NULL
                //checks shift and caps lock
              	} else if(SHIFTKEYACTIVE && CAPSLOCKKEYACTIVE){
                    //checks case of key that should be typed
                    if(isLetter){
                        if(terms[term_num].buffer_index < BUFFER_SIZE){
                        	  putc(scancodeLower[scancodeVal]); // Sends data from keyboard to coonsole
                        	  (terms[term_num].keyboard_buffer)[terms[term_num].buffer_index++]=scancodeLower[scancodeVal];
                        }
                    //checks case of key that should be typed
                    } else {
                         if(terms[term_num].buffer_index < BUFFER_SIZE){
                         	  putc(scancodeUpper[scancodeVal]); // Sends data from keyboard to coonsole
                        	  (terms[term_num].keyboard_buffer)[terms[term_num].buffer_index++]=scancodeUpper[scancodeVal];
                         }
                    }
                //checks case of key that should be typed
                } else if((CAPSLOCKKEYACTIVE && isLetter) || SHIFTKEYACTIVE){
                     if(terms[term_num].buffer_index < BUFFER_SIZE){
                     	    putc(scancodeUpper[scancodeVal]); // Sends data from keyboard to coonsole
                        	(terms[term_num].keyboard_buffer)[terms[term_num].buffer_index++]=scancodeUpper[scancodeVal];
                     }
                } else {
                     if(terms[term_num].buffer_index < BUFFER_SIZE){
                          putc(scancodeLower[scancodeVal]); // Sends data from keyboard to coonsole
                        	(terms[term_num].keyboard_buffer)[terms[term_num].buffer_index++]=scancodeLower[scancodeVal];
                     }
                }
            }
            //PSUDO CODE FOR POSSIBLE FUTURE USE
            //if(terms[term_num].entered ==1){
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
    // printf("Terminal read!\n");
    //enable the interrupt
    sti();
    //check if the terminal driver is open or not
    if(open_flag >= 8){
    	return 0;
    }
	//check if the parameters are valid
	if(buf == NULL || nbytes == 0){
		return 0;
	}
	//spin to wait the last interrupt ends
	while(!terms[run_term].entered);

	//clear the read flag
	terms[run_term].entered= 0;
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
		  if((terms[term_num].keyboard_buffer)[loop] == NULL || ((terms[term_num].keyboard_buffer)[loop - 1] == ENTER && loop > 0)){
			   break;
		  }
		  //copy into the new buffer
		  buf_uint[loop]= (terms[term_num].keyboard_buffer)[loop];
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
            (terms[term_num].keyboard_buffer)[i] = NULL;
         } else {
            if((terms[term_num].keyboard_buffer)[loop] == NULL || loop >= BUFFER_SIZE){
                nullSeen = 1;
            } else {
                overflowedChar++;
            }
            (terms[term_num].keyboard_buffer)[i]=(terms[term_num].keyboard_buffer)[loop++];
         }
      }
      terms[term_num].buffer_index=overflowedChar;

      // printf("Terminal read done\n");
     
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
  // printf("Terminal write!\n");
     //check if the terminal driver is open or not
    cli();
    if(open_flag >= 8){
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

    if(term_num == get_curr_pcb()->terminal){
      putc(buf_uint[loop]);
    }
    else{
      putc_sched(buf_uint[loop]);
    }
		
		bytes_written++;
    sti();
	}
  loop=0;
  // while((terms[run_term].keyboard_buffer)[loop] != NULL){
  //     if((terms[run_term].keyboard_buffer)[loop] == '\n'){
  //         putc((terms[run_term].keyboard_buffer)[loop++]);
  //         uint8_t bufNew[200];
  //         terminal_open(0);
  //         terminal_read(0, bufNew, 200);
  //         bytes_written += terminal_write(0, bufNew, 200);
  //         terminal_close(0);
  //         break;
  //     }
  //     putc((terms[run_term].keyboard_buffer)[loop++]);
  // }

  // printf("Terminal write end\n");
	return bytes_written;
}


int32_t init_term(){
  int i, j;
  for (j = 0; j < PT_SIZE; ++j){
    fakemem_pt[j] = EMPTY_PT_ENTRY;
  }

  for(i = 0; i < 3; i++){
    terms[i].buffer_index = 0;
    terms[i].keyboard_buffer[0] = '\0';
    terms[i].curs_x = 0;
    terms[i].curs_y = 0;
    terms[i].act_pid = -1;
    terms[i].init_ = 0;
    terms[i].entered = 0;
    fakemem_pt[i] = (((uint32_t)_32_MB + (i*FOUR_KB)) | PD_ATTRIB);
    sched_to_exec[i] = 0;
  }

  sched_to_exec[0] = 1;
  page_dir[33] = (((uint32_t)fakemem_pt & 0xFFFFF000) | PD_ATTRIB);
  flush_tlb();

  for (i = 0; i < 3; ++i){
    terms[i].vidmem = (uint8_t*)(_132_MB + (i*FOUR_KB));
    for (j = 0; j < NUM_ROWS * NUM_COLS; j++) {
        *(uint8_t *)(terms[i].vidmem + (j << 1)) = ' ';
        *(uint8_t *)(terms[i].vidmem + (j << 1) + 1) = ATTRIB;
    }
  }
  return 0;
}


void exec_shell_term(int term){
    set_cursor_pos(terms[term].curs_x, terms[term].curs_y);
    // term_num = term;
    terms[term].init_ = 1;
    execute((uint8_t*)"shell");
    // return 0;
}



/*
 ** int32_t switch_terminal(int32_t new_term)
 ** Inputs: new_term -- The new terminal to switch to
 ** nbytes: number of bytes to write to screen
 ** Return value: the number of bytes write correctly
 ** Description: The function we call to facilitate the switch between terminals;
 ** accomodates for executing the shell and saving/loading vidmem
 ** This function write nbytes bytes from the keyboard buffer to the screen.
*/
int32_t switch_terminal(int new_term){
  cli();
  int i = 0;
  int flag = 0;
  pcb_t* pcb_from;

  for(i = 0; i < 6; i++){
    if(!process_bit_map[i]){
      flag = 1;
    }
  }
  if((!flag) && (!terms[new_term].init_)){
    printf("No more free processes!\n");
    return -1;
  }

  if( new_term < 0 || new_term >= 3){
      return -1;
  }
  //check if we try to switch the same terminal
  if(new_term == term_num)
      return 0;

  // printf("Switching terminals..\n");
  //The repage

  // vmem_pt[0] = VMEM_P_ENTRY;
  // page_dir[VIDMAP_IDX] = ((((uint32_t) fakemem_pt & 0xFFFFF000) | PD_ATTRIB));
  // fakemem_pt[term_num] = terms[term_num].vidmem;
  //cli();

  //first we will save the old video memory
  memcpy((void*)terms[term_num].vidmem, (void*)VIDEO, FOUR_KB);
  //then we will load the new video memory
  memcpy((void*)VIDEO, (void*)terms[new_term].vidmem, FOUR_KB);

  // fakemem_pt[new_term] = VIDEO | PD_ATTRIB;

  term_num = new_term;
  //restore cursor pos
  set_cursor_pos(terms[new_term].curs_x, terms[new_term].curs_y);
  //Get relative PCBs in memory
  pcb_from = get_pcb(terms[term_num].act_pid);
  // printf("pcb_froms esp: %d, ebp: %d\n", pcb_from->esp, pcb_from->ebp );
  

  //If the terminal we switch to is not initialized, schedule it to execute on next PIT interrupts
  if(!terms[new_term].init_){
      //KEYBOARD HANDLER DOES NO PROCESS SWITCHING; Simply schedules terminal to start the first time
      sched_to_exec[new_term] = 1;
  }
  if(new_term == run_term){
         vmem_pt[0] = VMEM_P_ENTRY;
        page_dir[VIDMAP_IDX + 1] = ((((uint32_t) vmem_pt) & ADDR_BLACKOUT) | PDIR_MASK);  
  }
  else{
        vmem_pt[0] = (((uint32_t)_32_MB + (run_term*FOUR_KB)) | PDIR_MASK);
        page_dir[VIDMAP_IDX + 1] = ((((uint32_t) vmem_pt) & ADDR_BLACKOUT) | PDIR_MASK);  
  }
  flush_tlb();
  sti();
  return 0;
} 

int32_t is_sched_to_exec(uint32_t check_term){
  return sched_to_exec[check_term];
}
