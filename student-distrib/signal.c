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
	uint32_t user_esp, handler_addr, ret_add;
    // uint32_t* intx80ptr, moveax9ptr;
    // pcb_t* pcb_ptr;

 	
 	pcb_t* pcb_ptr = get_pcb(terms[run_term].act_pid);
					// asm volatile ("   \n\
    	// 				movl 64(%%ebp), %%eax \n\
    	// 				"
    	// 				:"=a"(user_esp)
    	// 				:
    	// 				:"cc"
    	// 			);
    				// printf("User esp: %x\n", user_esp);

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
					printf("we are here\n");
					asm volatile ("   \n\
    					movl 64(%%ebp), %%eax \n\
    					"
    					:"=a"(user_esp)
    					:
    					:"cc"
    				);

    				if(!user_esp){
    					printf("lol you fucked up\n");
    					return;
    				}
    				// printf("User esp: %x\n", user_esp);
                    ret_add = user_esp -12;
					
 					// pcb_ptr = get_curr_pcb();
  					handler_addr = (pcb_ptr->sig_data).hops[i];    				

					asm volatile ("   \n\
    					movl $0xCD089090, -4(%0)  \n\
    					movl $0x00909090, -8(%0)  \n\
    					movl $0xB8090000, -12(%0) \n\
    					movl 68(%%ebp), %%edi	  \n\
    					movl %%edi, -16(%0) 	  \n\
    					movl 64(%%ebp), %%edi	  \n\
    					movl %%edi, -20(%0) 	  \n\
    					movl 60(%%ebp), %%edi	  \n\
    					movl %%edi, -24(%0) 	  \n\
    					movl 56(%%ebp), %%edi	  \n\
    					movl %%edi, -28(%0) 	  \n\
    					movl 52(%%ebp), %%edi	  \n\
    					movl %%edi, -32(%0) 	  \n\
    					movl 48(%%ebp), %%edi	  \n\
    					movl %%edi, -36(%0) 	  \n\
    					movl 44(%%ebp), %%edi	  \n\
    					movl %%edi, -40(%0) 	  \n\
    					movl 40(%%ebp), %%edi	  \n\
    					movl %%edi, -44(%0) 	  \n\
    					movl 36(%%ebp), %%edi	  \n\
    					movl %%edi, -48(%0) 	  \n\
    					movl 32(%%ebp), %%edi	  \n\
    					movl %%edi, -52(%0) 	  \n\
    					movl 28(%%ebp), %%edi	  \n\
    					movl %%edi, -56(%0) 	  \n\
    					movl 24(%%ebp), %%edi	  \n\
    					movl %%edi, -60(%0) 	  \n\
    					movl 20(%%ebp), %%edi	  \n\
    					movl %%edi, -64(%0) 	  \n\
    					movl 16(%%ebp), %%edi	  \n\
    					movl %%edi, -68(%0) 	  \n\
    					movl 12(%%ebp), %%edi	  \n\
    					movl %%edi, -72(%0) 	  \n\
    					movl 8(%%ebp), %%edi	  \n\
    					movl %%edi, -76(%0) 	  \n\
    					movl %1, -80(%0) 		  \n\
                        movl %3, -84(%0)          \n\
    					movl %2, 52(%%ebp) 		  \n\
    					movl %0, %%edi 			  \n\
    					addl $-84, %%edi 		  \n\
    					movl %%edi, 64(%%ebp)	  \n\
    					"
    					:
    					:"r"(user_esp), "r"(i), "r"(handler_addr), "r"(ret_add)
    					:"edi", "cc"
    				);
    				//NO RET, IRET, or JMP
					//Still push return address to sigreturn on stack


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
