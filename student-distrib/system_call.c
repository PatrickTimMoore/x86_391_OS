#include "types.h"
#include "system_call.h"
#include "filesystem.h"
#include "lib.h"
#include "paging.h"
#include "keyboard_handler.h"
#include "x86_desc.h"

#define	MAX_BUF_SIZE 200
#define	FOUR		 4		//Hi peter	
#define	TWENNY		 20		//Hi peter	
#define DEL 		 0x7F
#define E   		 0x45
#define L 			 0x4C
#define F 			 0x46
#define PROCESS_NUM  2
#define PROC_PD_IDX 	  0x20
#define EIGHT_MB        0x800000
#define FOUR_MB         0x400000
#define ADDR_BLACKOUT	  0xFFFFF000
#define PROC_ATT		    0x183
#define EXEC_CPY_ADDR	  0x8048000
#define EIGHT_KB		    0x2000
#define USER_SP         0x8000000 + FOUR_MB - FOUR
#define USED_MASK       0x1
#define READ_MASK       0x2
#define WRITE_MASK      0x4


static int32_t process_bit_map[]={0, 0, 0, 0, 0, 0};
static int curr_pid = -1;
static jump_table_t terminal_jt = {terminal_open, terminal_read, terminal_write, terminal_close };



int32_t halt (uint8_t status){
	return -1;
}

int32_t execute (const uint8_t* command){
   int i, cmdstart, cmdend, cmdlen, byt;
   uint8_t cmd_buf[MAX_BUF_SIZE];
   uint8_t buf[MAX_BUF_SIZE];
   uint32_t entry_p;
   uint32_t* proc_load;
   pcb_t* pcb_ptr;


   /*PARSE THE ARGUMENT*/
   cmdlen = strlen((int8_t*)command);
  
  	// printf("Handed command: %s\n", command);
   // ./ls
   // cd ..
   //Skip all the spaces
   for(cmdstart = 0; cmdstart < cmdlen && command[cmdstart] == ' '; cmdstart++);

   //Find end of command
   for(cmdend = cmdstart; cmdend < cmdlen && command[cmdend] != ' ' && command[cmdend] != '\0'; cmdend++);
   
   // printf("cmdstart: %d, cmdend: %d\n", cmdstart, cmdend);
   //Copy over our cmd
   for(i = cmdstart; i < cmdend; i++){
   	cmd_buf[i-cmdstart] = command[i];
   }

   //Manually null terminate
   cmd_buf[i - cmdstart] = '\0';

   // printf("Executing %s...\n", cmd_buf);

   /*IS EXECUTABLE?*/
   open_file(cmd_buf);
   read_file(0, buf, FOUR);

   //check for (weird shit)ELF
   if(buf[0] != DEL &&
   	  buf[1] != E   &&
   	  buf[2] != L 	&&
   	  buf[3] != F){
   	printf("File is not executable!\n");
   	return -1;
   }

   read_file(0, buf, TWENNY+3);
   read_file(0, buf, FOUR);

   entry_p = (buf[0] << 8*3) + (buf[1] << 8*2) + (buf[2] << 8*1) + buf[3];

   int32_t process_num = -1;
   //create a new process
   for( i = 0; i<PROCESS_NUM; i++){
       if(process_bit_map[i] == 0){
       	    process_bit_map[i] = 1;
       	    process_num = i;
       	    break;
       }
   }
   //all the process are used
   if(process_num == -1){
       printf("All process are taken up");
       return -1;
   }
   /*SET UP PAGING*/
   //Set up the 4MB page for our process
   page_dir[PROC_PD_IDX] = ((EIGHT_MB + (process_num * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT;
   flush_tlb();

   // printf("EIGHT_MB: %x, process_num: %d, FOUR_MB: %x, ADDR_BLACKOUT: %x\n", EIGHT_MB, process_num, FOUR_MB, ADDR_BLACKOUT);
   // printf("Paged to %x with %x as index\n", PROC_PD_IDX << 22, ((EIGHT_MB + (process_num * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT);

   	/*PROCESS LOADER*/
   	proc_load = (uint32_t *)EXEC_CPY_ADDR;
	byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		byt = read_file(0, (void *)proc_load, 1);
		if(byt == -1){
			printf("%s\n", "Data read unsuccessful.");
			close_file(0);
			return -1;
		}
	}
	close_file(0);


	/*PROCESS CONTROL BLOCK*/

	//Set up pcb addressing
	pcb_ptr = (pcb_t*)(EIGHT_MB - (process_num + 1)*EIGHT_KB);

	//Assembly to get ebp0, esp0
  	asm volatile ("   \n\
    	movl %%esp, %%eax \n\
    	movl %%ebp, %%ebx \n\
    	"
    	:"=a"(pcb_ptr->esp0), "=b"(pcb_ptr->ebp0)
    	:
    	:"cc"
    );

    //SET PAGING
    pcb_ptr->next_idx = 0;
    pcb_ptr->pid0 = curr_pid;
    curr_pid = process_num;
    pcb_ptr->pid = process_num;

    //initalize the file descriptors for stdin and stdout
    (pcb_ptr->file_arr)[0].file_ops_ptr = &terminal_jt;
    (pcb_ptr->file_arr)[0].inode_num = -1;
    (pcb_ptr->file_arr)[0].file_pos = 0;
    (pcb_ptr->file_arr)[0].flags = 0;
    (pcb_ptr->file_arr)[0].flags |= USED_MASK;
    (pcb_ptr->file_arr)[0].flags |= READ_MASK;


    (pcb_ptr->file_arr)[1].file_ops_ptr = &terminal_jt;
    (pcb_ptr->file_arr)[1].inode_num = -1;
    (pcb_ptr->file_arr)[1].file_pos = 0;
    (pcb_ptr->file_arr)[1].flags = 0;
    (pcb_ptr->file_arr)[1].flags |= USED_MASK;
    (pcb_ptr->file_arr)[1].flags |= WRITE_MASK;

    //Switch TSS constants
    tss.esp0 = EIGHT_MB - process_num*EIGHT_KB - FOUR;
    tss.ss0 = KERNEL_DS;

    printf("Got here. \n");
    asm volatile ("   \n\
      mov %1, %%ds  \n\
      pushl %1      \n\
      pushl %3      \n\
      pushfl        \n\
      pushl %2      \n\
      pushl %0      \n\
      iret          \n\
      "
      :
      :"r"(entry_p), "r"(USER_DS), "r"(USER_CS), "r"(USER_SP)
      :"cc"
    );


   	return 0;
}

int32_t read (int32_t fd, void* buf, int32_t nbytes){
	return -1;
}

int32_t write (int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

int32_t open (const uint8_t* filename){
	dentry_t d;
	if(read_dentry_by_name(filename, &d) == -1){
		printf("Read dentry failed!\n");
		return -1;
	}
	return -1;
}

int32_t close (int32_t fd){
	return -1;
}

int32_t init_pcb(){
	return -1;
}

int32_t getargs (uint8_t* buf, int32_t nbytes){
	return -1;
}

int32_t vidmap (uint8_t** screen_start){
	return -1;
}

int32_t set_handler (int32_t signum, void* handler_address){
	return -1;
}

int32_t sigreturn (void){
	return -1;
}
