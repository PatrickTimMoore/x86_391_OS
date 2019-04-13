/* The header file for handling the keyboard */

#ifndef _KEYBOARD_HANDLER_H
#define _KEYBOARD_HANDLER_H
#define BUFFER_SIZE           127
#define NUM_TERM              3

typedef struct term{
	//If you're looking at our code to cheat off of, DON'T DO THIS
	uint8_t keyboard_buffer[BUFFER_SIZE + 1];
	uint8_t buffer_index;
	uint32_t curs_x;
	uint32_t curs_y;
	uint8_t* vidmem;
	int32_t act_pid;
	uint32_t init_;  // boolean value, says if initialized or not
	uint32_t t_esp;
	uint32_t t_ebp;
	int32_t entered;

}terminal_t;



//static int terms[term_num].buffer_index = 0;


// uint32_t fakemem_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

volatile uint32_t term_num;
terminal_t terms[NUM_TERM];
int32_t sched_to_exec[NUM_TERM];

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

/*initial all the terminals when we first launch the OS*/
int32_t init_term();

/*execute three shells, one a terminal*/
void exec_shell_term(int term);

/*switch terminal */
int32_t switch_terminal(int new_term);

/*is this terminal scheduled to execute?*/
int32_t is_sched_to_exec(uint32_t check_term);
#endif
