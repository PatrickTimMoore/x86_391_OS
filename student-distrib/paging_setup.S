#define ASM     1
#include "paging_setup.h"

#Set up masks to switch around bits
.data
    # CR3_CLEAR 	= 0xFFFFF000
    CR0_MASK 	= 0xE0000001 	#Enable Paging and Protection
    CR4_MASK 	= 0x00000010	#Enable 4MB pages
    CR4_CLEAR	= 0xFFFFFFDF    #Disable 36-bit addressing

.text
.globl pages_setup

# /* Function: extern void pages_setup(unsigned int*);
# *  Description: Puts Page Directory address in CR3, and
# *	 enables paging in CR0 and extended paging in CR4
# *  inputs: address to the first address of page directory
# *  ouputs: Appropriate modifications to cr0, cr4, and cr3
# *  clobbers: eax (as expected) and nothing else
# *  effects: Paging is now enabled and running
# */
pages_setup:
	pushl %ebp
	movl %esp, %ebp
	# Typical stack frame buildup

	movl 8(%esp), %eax 	#Pull address (argument) into eax
	# andl $CR3_CLEAR, %eax #Don't need technically because alignment
	movl %eax, %cr3 	#Move that first address into control register 3

	movl %cr4, %eax 	#Pull cr4 into eax (to modify)
	# movl $CR4_MASK, %ebx
	orl $CR4_MASK, %eax #Apply mask to turn on paging
	andl $CR4_CLEAR, %eax #Apply clear to turn off 36-bit addressing
	# movl $CR4_CLEAR, %ebx
	movl %eax, %cr4 	#Move the status into cr4

	movl %cr0, %eax 	#Get the value for eax
	# movl $CR0_MASK, %ebx
	orl $CR0_MASK, %eax #Enable paging and just in case protection 
	movl %eax, %cr0 	#Set them flags

	# Stack teardown
	leave
	ret
