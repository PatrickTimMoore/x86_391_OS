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


#define FOUR_KB               4096 
#define _32_MB                0x2000000
#define ADDR_BLACKOUT         0xFFFFF000
#define VMEM_PD_ENTRY_MASK    0x00000103
//Frequency
#define FREQ_10MS		11931

//Mode 3 (Square Wave Mode)
#define MODE_3			0x36

uint32_t vmem_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

void init_pit(){
	//We can assume our first running terminal is the first terminal
	run_term = 0;
	
	//Set to square wave generating mode
	outb(MODE_3, MODE_CMD_REG);

	//Set frequency to 1 tick every 10ms; need to send with 2 outbs
	outb((FREQ_10MS) & 0xff, DATA_PORT_0);
	outb((FREQ_10MS >> 8) & 0xff, DATA_PORT_0);

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
	sti();
}


void sched_switch(int term_from, int term_to){
	int pid_from = terms[term_from].act_pid;
	int pid_to = terms[term_to].act_pid;
	pcb_t* pcb_from;
	pcb_t* pcb_to;

	if(term_from < 0 || term_to < 0 || terms[term_from].act_pid < 0 || terms[term_to].act_pid < 0 ){
		// printf("Something's not right; term_from: %d, term_to: %d, act_pid_from: %d, act_pid_to: %d\n", term_from, term_to, terms[term_from].act_pid, terms[term_to].act_, );
		// return -1;
		return;
	}

	printf("Switch from term %d (process %d) to term %d (process %d); run_term is currently %d\n", term_from, terms[term_from].act_pid, term_to, terms[term_to].act_pid, run_term);
    // //remap the video memory to correct place
    // if(run_term != term_num){
    //     vmem_pt[184] = (((uint32_t)_32_MB + (run_term*FOUR_KB)) | VMEM_PD_ENTRY_MASK);
    //     page_dir[0] = ((((uint32_t) vmem_pt) & ADDR_BLACKOUT) | VMEM_PD_ENTRY_MASK);
    // }  

	printf("B\n");

	//Switch the run_term before we fuck with absolutely everything
    run_term = term_to;

	//Repage to pid_to's program load
	page_dir[PROC_PD_IDX] = ((EIGHT_MB + (pid_to * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT;
	flush_tlb();
	printf("C\n");

	//Get relative PCBs in memory
	pcb_from = get_pcb(pid_from);
	pcb_to = get_pcb(pid_to);
	printf("D\n");

	//Update the TSS
  	tss.esp0 = EIGHT_MB - (EIGHT_KB*pid_to) - FOUR;
  	tss.ss0 = KERNEL_DS;

	printf("E\n");

  	//Save esp, ebp
	//Assembly to get ebp0, esp0
  	asm volatile ("   \n\
    	movl %%esp, %0 \n\
    	movl %%ebp, %1 \n\
    	"
    	:"=r"(pcb_from->esp), "=r"(pcb_from->ebp)
      	:
    	:"cc"
    );
	printf("F\n");

    if(!terms[term_to].init_){
    	// return -1;
    	return;
    }

	printf("G\n");

	printf("from esp: %d, from ebp: %d\n", pcb_from->esp, pcb_from->ebp);	
	printf("to esp: %d, to ebp: %d\n", pcb_to->esp, pcb_to->ebp);	

    asm volatile ("   \n\
    	movl %0, %%esp 	  \n\
    	movl %1, %%ebp    \n\
    	"
    	:
      	:"r"(pcb_to->esp), "r"(pcb_to->ebp)
    	:"cc"
    );	
	//printf("HA\n");
	//sti();
    return;
}
