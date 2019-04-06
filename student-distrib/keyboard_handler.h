/* The header file for handling the keyboard */

#ifndef _KEYBOARD_HANDLER_H
#define _KEYBOARD_HANDLER_H

#define BUFFER_SIZE           127

typedef struct term{
	uint8_t keyboard_buffer[BUFFER_SIZE + 1];
	uint32_t curs_x;
	uint32_t curs_y;

}terminal_t;

/* function used to initialize/unmask the keyboard */
void init_keyboard();

/*handler function for keyboard */
void keyboard_interrupt();

/*this function opens the terminal driver*/
int32_t terminal_open(const uint8_t * filename);

/*this function closes the terminal driver*/
int32_t terminal_close(int32_t fd);

/*this function read nbytes from the keyboard buffer*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/*this function writes from the buf to the screen*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif

