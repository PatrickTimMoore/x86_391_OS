#include "lib.h"
#include "keyboard_handler.h"
#include "scheduling.h"
#include "i8259.h"
#include "paging.h"
#include "system_call.h"
#include "x86_desc.h"

//The IRQ line
#define PIT_IRQ_LINE	0

//Registers
#define DATA_PORT_0		0x40
#define DATA_PORT_1		0x41
#define DATA_PORT_2		0x42
#define MODE_CMD_REG	0x43

//Frequency
#define FREQ_10MS		11931

//Mode 3 (Square Wave Mode)
#define MODE_3			0x36

void init_pit(){
	//We can assume our first running terminal is the first terminal
	run_term = 0;
	
	//Set to square wave generating mode
	outb(MODE_3, MODE_CMD_REG);

	//Set frequency to 1 tick every 10ms; need to send with 2 outbs
	outb((FREQ_10MS) & 0xff, MODE_CMD_REG);
	outb((FREQ_10MS >> 8) & 0xff, MODE_CMD_REG);

	//Enable the irq line
	enable_irq(PIT_IRQ_LINE);
}

void pit_handler(){
	int next_term = run_term;
	//Send the EoI as the traditions dictate
	send_eoi(PIT_IRQ_LINE);

	do{
		next_term =  (next_term + 1) % 3;
		// needs to be here at the start because initally nothing is initialized
		if(next_term == run_term){
			sti();
			return;
		}
	}while(!(terms[next_term].init_));

	// context switch between curr run_term and next term, then update run_term
	sched_switch(run_term, next_term);
	run_term = next_term;

	//Unmask because we came in with and interrupt gate
	sti();
}


int32_t sched_switch(int term_from, int term_to){
	int pid_from = terms[term_from].act_pid;
	int pid_to = terms[term_to].act_pid;
	pcb_t* pcb_from;
	pcb_t* pcb_to;

	//Repage to pid_to's program load
	page_dir[PROC_PD_IDX] = ((EIGHT_MB + (pid_to * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT;
	flush_tlb();

	//Get relative PCBs in memory
	pcb_from = get_pcb(pid_from);
	pcb_to = get_pcb(pid_to);

	//Update the TSS
  	tss.esp0 = EIGHT_MB - (EIGHT_KB*pid_from) - FOUR;
  	tss.ss0 = KERNEL_DS;

  	//Save esp, ebp
	//Assembly to get ebp0, esp0
  	asm volatile ("   \n\
    	movl %%esp, %%eax \n\
    	movl %%ebp, %%ebx \n\
    	movl %0, %%esp 	  \n\
    	movl %1, %%ebp    \n\
    	"
    	:"=a"(pcb_from->esp), "=b"(pcb_from->ebp)
      	:"r"(pcb_to->esp), "r"(pcb_to->ebp)
    	:"cc"
    );

    return 0;
}
