#include "types.h"
#include "system_call.h"
#include "filesystem.h"
#include "lib.h"
#include "paging.h"
#include "keyboard_handler.h"
#include "rtc_handler.h"
#include "x86_desc.h"
#include "scheduling.h"
#include "signal.h"

#define	MAX_BUF_SIZE    200
#define	FOUR		        4		//Hi peter	
#define	TWENNY		      20		//Hi peter	
#define DEL 		        0x7F
#define E   		        0x45
#define L 			        0x4C
#define F 			        0x46
#define PROCESS_NUM     6
#define PROC_PD_IDX 	  0x20
#define EIGHT_MB        0x800000
#define FOUR_MB         0x400000
#define ADDR_BLACKOUT	  0xFFFFF000
#define PROC_ATT		    0x087
#define EXEC_CPY_ADDR	  0x8048000
#define EIGHT_KB		    0x2000
#define USER_SP         0x8000000 + FOUR_MB - FOUR
#define USED_MASK       0x1
#define READ_MASK       0x2
#define WRITE_MASK      0x4
#define EMPTY_PD_ENTRY  0x00000002
#define ENTRY_PT_OFFSET 24
#define ONE_TWENTY_EIGHT_MB 0x08000000
#define EMPTY_P_ENTRY   0x00000002
#define VMEM_P_ENTRY    0xB8007
#define ADDR_BLACKOUT   0xFFFFF000
#define PDIR_MASK       0x00000007
#define ONE             1
#define TWO             2
#define THREE           3
#define BYTE_SIZE       8
#define VIDMAP_IDX      33
#define FOUR_KB         4096
#define _32_MB          0x2000000


#define MASK_FOR_PCB    0xFFFFE000 

//For the video memory repaging
uint32_t vmem_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

//Bitmap for the processes
int32_t process_bit_map[PROCESS_NUM]={0, 0, 0, 0, 0, 0};
// static int (terms[term_num]).act_pid = -1;
//Our jump table
static jump_table_t terminal_jt = {terminal_open, terminal_read, terminal_write, terminal_close};
static jump_table_t rtc_jt = {rtc_open, rtc_read, rtc_write, rtc_close};
static jump_table_t dir_jt = {open_dir, read_dir, write_dir, close_dir};
static jump_table_t file_jt = {open_file, read_file, write_file, close_file};


/* Function: int32_t halt
*  Description: this function halts an entire process
*  inputs: status - the status of the procress
*  outputs: 0 for success and -1 for failure
*  effects: halts a user process
*/
int32_t halt (uint8_t status){
  // process control block pointer - from 4MB to 8MB, each pcb is sequential
  pcb_t* pcb_ptr = (pcb_t*)(EIGHT_MB - ((terms[run_term]).act_pid + 1)*EIGHT_KB);
  // parent pid
  int32_t par_pid = pcb_ptr->pid0;
  // loop ctr - used to close files of process
  int i;

 // closes all files not including stdin and stdout  
  for (i = 2; i < FILES_NUM; ++i)
    close(i);

 
  // closes the terminal driver - shared by stdin and out
  (*(((pcb_ptr->file_arr)[0].file_ops_ptr)->close))(0);

  // close stdin
  (pcb_ptr->file_arr)[0].file_ops_ptr = NULL;
  (pcb_ptr->file_arr)[0].inode_num = -1;
  (pcb_ptr->file_arr)[0].file_pos = 0;
  (pcb_ptr->file_arr)[0].flags &= 0;

  // close stdout
  (pcb_ptr->file_arr)[1].file_ops_ptr = NULL;
  (pcb_ptr->file_arr)[1].inode_num = -1;
  (pcb_ptr->file_arr)[1].file_pos = 0;
  (pcb_ptr->file_arr)[1].flags &= 0;

  // process bit map, sets process to not running status
  process_bit_map[(terms[run_term]).act_pid] = 0;
  //Set up the 4MB page for our parent process (or depage)
  (terms[run_term]).act_pid = par_pid;
   // if there is no parent, return to shell
   if(par_pid < 0){
       execute((uint8_t*)"shell");
   }
   else{
      // printf("Halt: Paging to %x\n", (EIGHT_MB + (par_pid * FOUR_MB)));
       // remaps paging to physical memory of start of the program in userspace
      page_dir[PROC_PD_IDX] = ((EIGHT_MB + (par_pid * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT;
      flush_tlb();
   }
   // printf("Repaged!\n");

  // task stack segment
  // sets parents kernel stack location
  tss.esp0 = EIGHT_MB - (EIGHT_KB*par_pid) - FOUR;
  // sets parents data segment
  tss.ss0 = KERNEL_DS;

  // printf("Bookkeeping set\n");

  // critical section end
  sti();
  // expands 8bit arg from BL into the 32bit return value 
  //     to the parent program's execute syscall
  asm volatile ("     \n\
  	  movzbl %2, %%eax  \n\
  	  movl %0, %%esp \n\
      movl %1, %%ebp \n\
      jmp EXEC_RET      \n\
      "
      :
      :"r"(pcb_ptr->esp0), "r"(pcb_ptr->ebp0),"r"(status)
      :"%eax"
  );

  // never gets to here, needed to suppress warnings
	return 0;
}



/* 
 *  Function: int32_t execute
 *  Description: This function executes a program
 *  Inputs: command - the command to run
 *  Outputs: 0 for success and -1 for failure
 *  Effects: 
 */

int32_t execute (const uint8_t* command){
   //If we're off just drop the request
   // if(run_term != term_num){
      // return;
   // }
   cli();

   //Signals are good to go if they haven't yet
   if(!signals_ready){
    signals_ready = 1;
   }
   
   int i, cmdstart, cmdend, cmdlen, byt, offset;
   dentry_t d;
   uint8_t cmd_buf[MAX_BUF_SIZE];
   uint8_t buf[MAX_BUF_SIZE];
   uint32_t entry_p;
   uint8_t* proc_load;
   pcb_t* pcb_ptr;
   char test_s[MAX_BUF_SIZE];
   int argbound;

   if(!command){return -1;}
   // printf("Execute!\n");
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

    // printf("Parsing arguments...\n");
    // printf("cmdend: %d, cmdlen %d, char at cmdend: %c\n", cmdend, cmdlen, command[cmdend] );
    // if(command[cmdend] == '\n'){
    // find the start of the arguments
    for(cmdstart = cmdend; (cmdstart < cmdlen) && (command[cmdstart] == ' '); cmdstart++);

    // printf("new cmdstart: %d\n", cmdstart);
    // if there are arguments 
    if(command[cmdstart] != '\0' && cmdstart < cmdlen){
      // printf("We have arguments!\n");
      // finds the end of the list of arguments, the true command end	    
      for(cmdend = cmdstart; cmdend < cmdlen && command[cmdend] != ' ' && command[cmdend] != '\0'; cmdend++);
      // copies over args into buffer
      for(i = cmdstart; i < cmdend; i++){
        // printf("Copying %c\n", command[i]);
        test_s[i-cmdstart] = command[i];
      }
      //Manually null terminate
      test_s[i - cmdstart] = '\0';

	    // find length of arguments
      argbound = cmdend - cmdstart;
    }
    else{
        // no arguments
        test_s[0] = '\0';
        argbound = 0;
    }
    
    // offset for reading the file
    offset = 0;
    // printf("Executing %s...\n", cmd_buf);


   /*Checks to see if file exists*/
   /*IS EXECUTABLE?*/
   // open_file(cmd_buf);
   // read_file(0, buf, FOUR);
   // printf("Reading dentry by name\n");
   if(read_dentry_by_name(cmd_buf, &d) == -1){
    printf("Executable \"%s\" not found; run_term: %d.\n", cmd_buf, run_term);
    return -1;
   }
   // printf("Reading first 4 bytes\n");
   read_data(d.inode_num, 0, buf, FOUR);


   // printf("Check _ELF\n");
   //check for (weird shit)ELF
   if(buf[0] != DEL &&
   	  buf[ONE] != E   &&
   	  buf[TWO] != L 	&&
   	  buf[THREE] != F){
   	printf("File is not executable!\n");
   	return -1;
   }

   // read_file(0, buf, TWENNY);
   // read_file(0, buf, FOUR);

    // reads the entry point data for the program we are loading
    read_data(d.inode_num, ENTRY_PT_OFFSET, buf, FOUR);

    // find the start/entry point of the program
    entry_p = (buf[THREE] << BYTE_SIZE*THREE) + (buf[TWO] << BYTE_SIZE*TWO) + (buf[ONE] << BYTE_SIZE*ONE) + buf[0];
   // printf("%x\n", entry_p);

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
       printf("All process are taken up\n");
       return -1;
   }
   // num_p++;
   /*SET UP PAGING*/
   //Set up the 4MB page for our process
   // printf("Paged to %x\n", (EIGHT_MB + (process_num * FOUR_MB)) );
   page_dir[PROC_PD_IDX] = ((EIGHT_MB + (process_num * FOUR_MB)) & ADDR_BLACKOUT) + PROC_ATT;
   flush_tlb();

  /*PROCESS LOADER*/
  // program loader, the address we need to copy into
  proc_load = (uint8_t *)EXEC_CPY_ADDR;

  // close_file(0);
  // open_file(cmd_buf);

  // printf("Got here 0\n");
  byt = 1;
	// printf("Opened file\n", byt);
	while(byt){
		// printf("Reading from file\n", byt);
		// byt = read_file(0, (void *)proc_load, 1);
    byt = read_data(d.inode_num, offset, (void *)proc_load, 1);
		if(byt == -1){
			printf("%s\n", "Data read unsuccessful.");
			// close_file(0);
			return -1;
		}
    offset += byt;
    proc_load++;
	}
	// close_file(0);

  // printf("Got here 1\n");

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
    // pcb_ptr->next_idx = 0;
    // pcb_ptr->pid0 = (terms[term_num]).act_pid;
    // (terms[term_num]).act_pid = process_num;
    // sets parent pid
    pcb_ptr->pid0 = (terms[term_num]).act_pid;
    // changes active pid
    (terms[term_num]).act_pid = process_num;
    pcb_ptr->pid = process_num;
    pcb_ptr->terminal = term_num;

    // Initializes signal information
    for (i = 0; i < NUM_SIGS; ++i){
      (pcb_ptr->sig_data).sig_stat[i] = 0;
      (pcb_ptr->sig_data).hops[i] = 0;
    }

    // copies the command arguments
    for(i = 0; i < argbound; i++){
      (pcb_ptr->argbuf)[i] = test_s[i];
    }
    
    //change the scheduling terminal when a new process execute
    // run_term = term_num;

    //Self null-terminate
    (pcb_ptr->argbuf)[argbound] = '\0';

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

    (*(((pcb_ptr->file_arr)[0].file_ops_ptr)->open))(0);

    //Switch TSS constants
    tss.esp0 = EIGHT_MB - process_num*EIGHT_KB - FOUR;
    tss.ss0 = KERNEL_DS;

    // printf("Got here. \n");
    // critical section ends
    sti();

    // assembly linkage
    asm volatile ("   \n\
      mov %1, %%ds  \n\
      pushl %1      \n\
      pushl %3      \n\
      pushfl        \n\
      pushl %2      \n\
      pushl %0      \n\
      iret          \n\
      EXEC_RET:     \n\
      leave         \n\
      ret           \n\
      "
      :
      :"r"(entry_p), "r"(USER_DS), "r"(USER_CS), "r"(USER_SP)
      :"cc"
    );

    // for suppressing warnings
   	return 0;
}


/* Function: int32_t read
*  Description: this function reads data from the keyb, file, device (RTC), or dir
*  Inputs: 
*  		fd - file descriptor
*		buf - contains data read
*		nbytes - length of buf, number of bytes read
*  Outputs: 0 for success and -1 for failure
*  Effects: Reads from file
*/

int32_t read (int32_t fd, void* buf, int32_t nbytes){
  // printf("Syscall: read\n");
  pcb_t* pcb_ptr;  // process control block ptr

   // check of fd
  if(fd < 0 || fd >= FILES_NUM){
    printf("read: Go fix your index you dumb fuck\n");
    return -1;
  }
 
  // gets the pointer in kernelspace
  pcb_ptr = get_curr_pcb();
  if((((pcb_ptr->file_arr)[fd].flags & USED_MASK) == 0) ||
   (((pcb_ptr->file_arr)[fd].flags & READ_MASK) == 0)){
    printf("read (fd:%d): I can't read from this shit go away\n", fd);
    return -1;
  }
  
  // fxn pointer going to specific file's read function and returns its value
	return (*(((pcb_ptr->file_arr)[fd].file_ops_ptr)->read))(fd, buf, nbytes);
}





/* 
*  Function: int32_t write
*  Description: this function writes data from buf to the screen 
*  	or sets rtc freq. Writing to files always return failue
*  Inputs: 
*  		fd - file descriptor
*		buf - contains data to write
*		nbytes - length of buf, number of bytes to write
*  Outputs: 0 for success and -1 for failure
*  Effects: Writes to term or device
*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
  // printf("Syscall: write (%d)\n", fd);
  pcb_t* pcb_ptr;
  // check of fd
  if(fd < 0 || fd >= FILES_NUM){
    printf("write: Go fix your index you dumb fuck\n");
    return -1;
  }

  // gets the pointer in kernelspace
  pcb_ptr = get_curr_pcb();
  if((((pcb_ptr->file_arr)[fd].flags & USED_MASK) == 0) ||
   (((pcb_ptr->file_arr)[fd].flags & WRITE_MASK) == 0)){
    printf("write: I can't write to this shit go away\n");
    return -1;
  }
  // fxn pointer going to specific file's write function and returns its value
  return (*(((pcb_ptr->file_arr)[fd].file_ops_ptr)->write))(fd, buf, nbytes);
}





/* Function: int32_t open (const uint8_t* filename)
*  Description: This is a system call
*  inputs: 
*  ouputs: 0 for success and -1 for failure
*  effects: 
*/
int32_t open (const uint8_t* filename){
  // printf("Syscall: open\n");
  int i;
	dentry_t d;
  pcb_t* pcb_ptr;
	// attempts to get dentry info
  if(read_dentry_by_name(filename, &d) == -1){
		printf("open: Read dentry failed!\n");
		return -1;
	}

  // can't open a file if no process running
  if((terms[run_term]).act_pid == -1){
    printf("open: No process running!\n");
    return -1;
  }
  // gets pcb pointer
  pcb_ptr = (pcb_t*)(EIGHT_MB - ((terms[run_term]).act_pid + 1)*EIGHT_KB);
  // check to see if have space to open file
  for (i = 2; i < FILES_NUM; ++i){
      if(((pcb_ptr->file_arr)[i].flags && USED_MASK) == 0){
        break;
      }
  }
  // i equal to FILES_NUM means  no space to open a file
  if(i == FILES_NUM){
    printf("open: File array is full!\n");
    return -1;
  }
  // sets file descriptor information
  switch(d.file_type){
    case 0:  // for rtc
      (pcb_ptr->file_arr)[i].file_ops_ptr = &rtc_jt;
      (pcb_ptr->file_arr)[i].inode_num = -1;
      (pcb_ptr->file_arr)[i].file_pos = 0;
      (pcb_ptr->file_arr)[i].flags = 0;
      (pcb_ptr->file_arr)[i].flags |= USED_MASK;
      (pcb_ptr->file_arr)[i].flags |= READ_MASK;
      (pcb_ptr->file_arr)[i].flags |= WRITE_MASK;

      break;
    case 1:  // for directory
      // printf("Opening dir...\n");
      (pcb_ptr->file_arr)[i].file_ops_ptr = &dir_jt;
      (pcb_ptr->file_arr)[i].inode_num = -1;
      (pcb_ptr->file_arr)[i].file_pos = 0;
      (pcb_ptr->file_arr)[i].flags = 0;
      (pcb_ptr->file_arr)[i].flags |= USED_MASK;
      (pcb_ptr->file_arr)[i].flags |= READ_MASK;

      break;
    case 2:  // for regular file
      (pcb_ptr->file_arr)[i].file_ops_ptr = &file_jt;
      (pcb_ptr->file_arr)[i].inode_num = d.inode_num;
      (pcb_ptr->file_arr)[i].file_pos = 0;
      (pcb_ptr->file_arr)[i].flags = 0;
      (pcb_ptr->file_arr)[i].flags |= USED_MASK;
      (pcb_ptr->file_arr)[i].flags |= READ_MASK;

      break;
    default:
      printf("open: %d is not valid filetype, go fix this\n", d.file_type);
      break;
  }
  // calls file's open fxn
  (*(((pcb_ptr->file_arr)[i].file_ops_ptr)->open))(filename);

  // returns the file descriptor index
	return i;
}


/* 
*  Function: int32_t close
*  Description: this function closes a file
*  Inputs: fd - file descriptor index
*  Outputs: 0 for success and -1 for failure
*  Effects: Closes file
*/

int32_t close (int32_t fd){
  pcb_t* pcb_ptr;

  if(fd < 0 || fd >= FILES_NUM){
    printf("close: Go fix your index you dumb fuck\n");
    return -1;
  }
  if(fd == 0 || fd == 1){
    printf("close: How the fuck will you run shit if you close stdin or stdout!\n");
    return -1;
  }
  // gets pcb ptr
  pcb_ptr = (pcb_t*)(EIGHT_MB - ((terms[run_term]).act_pid + 1)*EIGHT_KB);
  // checks to see if file is even open
  if(!((pcb_ptr->file_arr)[fd].flags & USED_MASK)){
    return -1;
  }
  // cloes the file, clears flags and variables
  (*(((pcb_ptr->file_arr)[fd].file_ops_ptr)->close))(fd);
  (pcb_ptr->file_arr)[fd].file_ops_ptr = NULL;
  (pcb_ptr->file_arr)[fd].inode_num = -1;
  (pcb_ptr->file_arr)[fd].file_pos = 0;
  (pcb_ptr->file_arr)[fd].flags &= 0;

	return 0;
}


/* 
*  Function: int32_t getargs
*  Description: this function gets a programs arguments
*  Inputs: 
*  		buf - buffer to return arguments in
*  		nbytes - length of buf, bumber of bytes to write
*  Outputs: 0 for success and -1 for failure
*  Effects: 
*/

int32_t getargs (uint8_t* buf, int32_t nbytes){
  //get the length of argument
  int arglen = strlen((int8_t*)(get_curr_pcb()->argbuf));

  // check if buf is valid
  if(!buf){return -1;}

  // copies argbuf to buf
  if(arglen && arglen < nbytes){
      //use helper function to copy arguments
      memcpy(buf, get_curr_pcb()->argbuf, nbytes);
      return 0;
  }
  else
      return -1;

}


/* 
*  Function: int32_t vidmap
*  Description: this function calls the text-mode video memory into user space
*  	at a pre-set virtual address
*  Inputs: screen_start - ptr to ptr of start of screen
*  Ouputs: 0 for success and -1 for failure
*  Effects: 
*/

int32_t vidmap (uint8_t** screen_start){
  int i;

  // check to see if start is valid
  if((uint32_t)screen_start < ONE_TWENTY_EIGHT_MB || (uint32_t)screen_start >= ONE_TWENTY_EIGHT_MB + FOUR_MB){
    return -1;
  }

  //Choosing 136 MB to load prog into
  *screen_start = (uint8_t*)ONE_TWENTY_EIGHT_MB + FOUR_MB + FOUR_MB ;
  for (i = 0; i < PT_SIZE; ++i){
    vmem_pt[i] = EMPTY_P_ENTRY;
  }

  //map the first entry of our page table to video memory
  vmem_pt[0] = VMEM_P_ENTRY;
  // sets page dir entry to start of vmem_pt and address
  page_dir[VIDMAP_IDX + 1] = ((((uint32_t) vmem_pt) & ADDR_BLACKOUT) | PDIR_MASK);
  flush_tlb();
  //return the mapped address to video memory
  return (int32_t)*screen_start;
}


/* 
*  Function: int32_t set_handler
*  Description: Does nothing. Related to signal handling LOL NOT ANYMORE
*  Inputs: 
*  		signum - signal number
*  		handler_address - address of handler
*  Outputs: 0 for success and -1 for failure
*  Effects: Nothing so far
*/

int32_t set_handler (int32_t signum, void* handler_address){
	pcb_t* pcb_ptr;
  if(signum < 0 || signum >= NUM_SIGS){
    //GET THE FUCK OUTTA HERE
    return -1;
  }

  if(!handler_address){
    // printf("Invalid pointer; not in userspace\n");
    return -1;
  }

  pcb_ptr = get_curr_pcb();
  (pcb_ptr->sig_data).hops[signum] = (uint32_t)handler_address;
  return 0;
}


/* 
*  Function: int32_t setreturn
*  Description: Does nothing. Related to signal handling
*  Inputs: Nothing
*  Outputs: 0 for success and -1 for failure
*  Effects: Nothing so far
*/

int32_t sigreturn (void){
  // printf("Sig Return: Yeet.\n");
  int i;
  uint32_t user_esp, eax;
  asm volatile ("   \n\
      movl 60(%%ebp), %%eax \n\
      "
      :"=a"(user_esp)
      :
      :"cc"
  );

  i = *((int *)user_esp);
  user_esp += 4;
              // movl 24(%0), %%ebx      \n\

  asm volatile ("                     \n\
              movl 60(%0), %%edi      \n\
              movl %%edi, 64(%%ebp)   \n\
              movl 56(%0), %%edi      \n\
              movl %%edi, 60(%%ebp)   \n\
              movl 52(%0), %%edi      \n\
              movl %%edi, 56(%%ebp)   \n\
              movl 48(%0), %%edi      \n\
              movl %%edi, 52(%%ebp)   \n\
              movl 44(%0), %%edi      \n\
              movl %%edi, 48(%%ebp)   \n\
              movl 40(%0), %%edi      \n\
              movl %%edi, 44(%%ebp)   \n\
              movl 36(%0), %%edi      \n\
              movl %%edi, 40(%%ebp)   \n\
              movl 32(%0), %%edi      \n\
              movl %%edi, 36(%%ebp)   \n\
              movl 28(%0), %%edi      \n\
              movl %%edi, 32(%%ebp)   \n\
              movl 20(%0), %%edi      \n\
              movl %%edi, 28(%%ebp)   \n\
              movl 16(%0), %%edi      \n\
              movl %%edi, 24(%%ebp)   \n\
              movl 12(%0), %%edi      \n\
              movl %%edi, 20(%%ebp)   \n\
              movl 8(%0), %%edi       \n\
              movl %%edi, 16(%%ebp)   \n\
              movl 4(%0), %%edi       \n\
              movl %%edi, 12(%%ebp)   \n\
              movl (%0), %%edi        \n\
              movl %%edi, 8(%%ebp)    \n\
              movl 24(%0), %%eax      \n\
              "
              :"=a"(eax)
              :"r"(user_esp)
              :"edi", "cc"
            );

  // printf("Got past THAT shit\n");
  (get_pcb(terms[run_term].act_pid)->sig_data).sig_stat[i] = 0;

  return eax;
}



/* 
*  Function: int32_t get_curr_pcb
*  Description: calculates current pcb locations
*  Inputs: Nothing
*  Outputs: pcb location
*  Effects: Nothing so far
*/

extern pcb_t* get_curr_pcb(){
  
  pcb_t* ptr;
  //extract current pcb from esp
	asm volatile("				   \n\
				andl %%esp, %%eax  \n\
				"
				:"=a"(ptr)
				:"a"(MASK_FOR_PCB)
				:"cc"
				);
	return ptr;
}


/* 
*  Function: int32_t get_curr_pid
*  Description: returns curr pid for external functions
*  Inputs: Nothing
*  Outputs: Current pid number
*  Effects: Nothing so far
*/

extern int32_t get_curr_pid(){
  // return (terms[term_num]).act_pid;
  return (terms[run_term]).act_pid;
}



/* 
*  Function: int32_t get_pcb
*  Description: calculates pcb location of the gpid
*  Inputs: gpid - global process id 
*  Outputs: pcb location
*  Effects: Nothing so far
*/

extern pcb_t* get_pcb(int gpid){
  //return the pcb with gpid
  return (pcb_t*)(EIGHT_MB - (gpid + 1)*EIGHT_KB);

}
