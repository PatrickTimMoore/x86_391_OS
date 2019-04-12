#ifndef SCHED_H
#define SCHED_H

#include "types.h"

#define	MAX_BUF_SIZE    	200
#define	FOUR		        4		//Hi peter	
#define	TWENNY		      	20		//Hi peter	
#define DEL 		        0x7F
#define E   		        0x45
#define L 			        0x4C
#define F 			        0x46
#define PROCESS_NUM     	6
#define PROC_PD_IDX 	  	0x20
#define EIGHT_MB       		0x800000
#define FOUR_MB         	0x400000
#define ADDR_BLACKOUT	  	0xFFFFF000
#define PROC_ATT		    0x087
#define EXEC_CPY_ADDR	  	0x8048000
#define EIGHT_KB		    0x2000
#define USER_SP         	0x8000000 + FOUR_MB - FOUR
#define USED_MASK       	0x1
#define READ_MASK       	0x2
#define WRITE_MASK      	0x4
#define EMPTY_PD_ENTRY  	0x00000002
#define ENTRY_PT_OFFSET 	24
#define ONE_TWENTY_EIGHT_MB 0x08000000
#define EMPTY_P_ENTRY   	0x00000002
#define VMEM_P_ENTRY    	0xB8007
#define ADDR_BLACKOUT   	0xFFFFF000
#define PDIR_MASK       	0x00000007
#define ONE             	1
#define TWO             	2
#define THREE           	3
#define BYTE_SIZE       	8
#define VIDMAP_IDX      	33

int32_t run_term;

void init_pit();
void pit_handler();
// void schedule();
void sched_switch(int term_from, int term_to);

#endif
