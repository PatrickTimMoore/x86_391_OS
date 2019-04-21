#include "signal.h"
#include "types.h"
#include "system_call.h"
#include "keyboard_handler.h"
#include "scheduling.h"
#include "lib.h"

#define NUM_SIGS		5
#define ADDR_TO_ESP 	15*4;
#define INTx80 			0xb8090000
#define MOVEAX9 		0xCD800000

// void init_signals(void){
// 	signals_ready = 1;
// }

int signals_ready = 0;

void kill_task(void){
	halt(0);
}

void ignore(void){
	//fuck knows
}

void sig_pending(void){
	if(!(signals_ready == 1)){return;}
	int i;
	uint32_t user_esp;
 	pcb_t* pcb_ptr = get_pcb(terms[run_term].act_pid);
					asm volatile ("   \n\
    					movl 60(%%ebp), %%eax \n\
    					"
    					:"=a"(user_esp)
    					:
    					:"cc"
    				);
    				printf("User esp: %x\n", user_esp);

	// void* hand_ptr;
	for (i = 0; i < NUM_SIGS; ++i){
		//Are we at a pending signal?
		if((pcb_ptr->sig_data).sig_stat[i] == -1){
				// printf("Servicing Signal %d\n", i);
				(pcb_ptr->sig_data).sig_stat[i] = 1; //Set to executing
				//User or kernel handler?
				if (!(pcb_ptr->sig_data).hops[i]){ //kernel handler
					if(i == 0 || i == 1 || i == 2){
						kill_task();
					}
					else{
						ignore();
					}
				}
				else{ //user handler
				// 	asm volatile ("   \n\
    // 					movl 68(%%ebp), %%eax \n\
    // 					"
    // 					:"=a"(user_esp)
    // 					:
    // 					:"cc"
    // 				);
    // 				// printf("User esp: %x\n", user_esp);

				}
		}
	}
}

int32_t raise_sig(uint32_t signum){
  pcb_t* pcb_ptr;
  if(signum < 0 || signum >= NUM_SIGS){
    //GET THE FUCK OUTTA HERE
    return -1;
  }
  pcb_ptr = get_pcb(terms[run_term].act_pid); 

  if((pcb_ptr->sig_data).sig_stat[signum] == 1){
  	//Cannot re-raise an executing signal
  	return -1;
  }

  (pcb_ptr->sig_data).sig_stat[signum] = -1;
  return 0;
}
