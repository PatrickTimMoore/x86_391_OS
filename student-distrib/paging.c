#include "paging.h"
#include "paging_setup.h"
#include "types.h"
#include "lib.h"

// #define PD_SIZE 1024
// #define PT_SIZE 1024
// #define PAGE_SIZE 4096
// #define KERNEL_PAGE 1

// This sets the following entry:
    //   Supervisor: Only kernel can access this
    //   Write Enabled: Read/Write activated
    //   Not Present: The page table is not active
#define EMPTY_PD_ENTRY 0x00000002

// This sets the following entry:
    //   Supervisor: Only kernel can access this
    //   Write Enabled: Read/Write activated
    //   Not Present: The page table is not active
#define EMPTY_PT_ENTRY 0x00000002

//For when we only need first 20 bits
#define ADDR_BLACKOUT 0xFFFFF000

//NO WRITE THROUGH RN, ONLY WRITE BACK

//The page entry for the (preloaded) kernel code.
    //  Page Size: Set, this a 4MB page
    //  Supervised: Only kernel can write (for now)
    //  Write Enabled: Of fucking course, we're writing to it as we speak
    //  Global: Yee, but shouldn't make a difference
    //  Present: Boi
#define KERNEL_PD_ENTRY 0x00400183

//The entry for the page table (in page directory) for the video memory
    //  NOT Page Size: This a table
    //  Unsupervised: Your dog can write to this if they want
    //  Write Enabled: Yeah, we want to change the actual screen
    //  NOT Global: Cannot cache this, this is memory-mapped
    //  Present: Yeah we want screen to be there
#define VMEM_PD_ENTRY_MASK 0x00000103

//The entry IN the page table for the video memory
    //  NOT Page Size: This a table
    //  Unsupervised: Your dog can write to this if they want
    //  Write Enabled: Yeah, we want to change the actual screen
    //  NOT Global: Cannot cache this, this is memory-mapped
    //  Present: Yeah we want screen to be there
#define VMEM_PT_ENTRY 0x000B8003

//For easier non-magic
#define B8_IN_DEC 184 //Yee

//Our video memory page table
uint32_t first_pt[PT_SIZE] __attribute__((aligned(PAGE_SIZE)));

/* Function: int init_paging
*  Description: this function is used to initialize the paging directory, 
*   along with initial entries
*  inputs: none
*  ouputs: 0 for success and -1 for failure
*  effects: create a page directory, initialize entries
*/
int init_page_dir(){
  //For indexing
  int i;
  //Clear out the page table entries, BUT set them to r/w enabled
  for (i = 0; i < PD_SIZE; ++i){
    page_dir[i] = EMPTY_PD_ENTRY;
  }
  return 0;
}

/* Function: int init_kernel_page
*  Description: Set up a 4MB page for the Kernel
*  inputs: none
*  ouputs: 0 for success and -1 for failure
*  effects: create a 4MB page for the kernel
*/
int init_kernel_page(){
  //Set up a 4MB page, see above for details
  page_dir[KERNEL_PAGE] = KERNEL_PD_ENTRY;
  return 0;  
}


/* Function: int init_vmem
*  Description: Set up a table and space for video memory in it
*  inputs: none
*  ouputs: 0 for success and -1 for failure
*  effects: create a table and space for vmem
*/
int init_vmem(){
  //For indexing
  int i;
  
  //Clear out the page table entries, BUT set them to r/w enabled
  for (i = 0; i < PT_SIZE; ++i){
    first_pt[i] = EMPTY_PT_ENTRY;
  }
  //Put the video memory page in page directory
  first_pt[B8_IN_DEC] = VMEM_PT_ENTRY;

  //Put the page in page directory
  page_dir[0] = ((((uint32_t) first_pt) & ADDR_BLACKOUT) | VMEM_PD_ENTRY_MASK); 
  return 0;  
}


/* Function: int set_paging()
*  Description: Modify control registers to enable safe paging
*  inputs: none (page_dir pointer paritally)
*  ouputs: 0 for success and -1 for failure
*  effects: Paging now on
*/
int set_paging(){
  //Invoke the assembly function
  pages_setup(page_dir);
  return 0;
}

/* Function: flush_tlb
*  Description: Flushes Translation Lookaside Buffer (TLB)
*  inputs: none 
*  ouputs: none (void)
*  effects: Clobbers eax, hard flushes TLB
*/
void flush_tlb(){
  asm volatile ("   \n\
    movl %cr3, %eax \n\
    movl %eax, %cr3");
}

