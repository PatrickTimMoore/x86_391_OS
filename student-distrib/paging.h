#ifndef PAGING_INIT_H
#define PAGING_INIT_H

#include "types.h"

//Sizing constants
#define PD_SIZE 1024
#define PT_SIZE 1024
#define PAGE_SIZE 4096
#define KERNEL_PAGE 1

//Our global page directory
uint32_t page_dir[PD_SIZE] __attribute__((aligned(PAGE_SIZE)));

//These 4 are meant to be called in order to enable paging
/**
  *this function helps to initialize the paging directory with empty entries
 **/
int init_page_dir();
/*Allocates a 4MB page for the kernel code*/
int init_kernel_page();
/*Allocates memory for video memory*/
int init_vmem();
/*Set up the control registers to enable paging*/
int set_paging();
// Flushes Translation Lookaside Buffer
void flush_tlb();


#endif
