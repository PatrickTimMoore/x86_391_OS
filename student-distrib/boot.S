# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"
#include "idt_init.h"

.data
    PESET = 0x1B       #Set the corresponding bits in cr0 or leave them as is
    #Set Protection Enable, Monitor Coprocessor, Task Switched, Extension Type NOTE: NO PAGING ENABLED YET
.text

    # Multiboot header (required for GRUB to boot us)
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

# Entrypoint to the kernel
.globl start, _start

.align 4
start:
_start:
    # Make sure interrupts are off
    cli
    jmp     continue

continue:
    # Load the GDT
    lgdt gdt_desc_ptr
    #initialize the IDT
    call init_idt
    #Load the IDT
    lidt idt_desc_ptr

    #Set condition code 0 register
    movl %cr0, %ebx #Load cr0
    orl  PESET, %ebx #Check above for flags we set
    movl %ebx, %cr0 #Store it back in

    # Load CS with the new descriptor value
    ljmp    $KERNEL_CS, $keep_going

keep_going:
    # Set up ESP so we can have an initial stack
    movl    $0x800000, %esp         #MAGIC NUMBER IN GIVEN CODE DAWG GET YO SHIT TOGETHER

    # Set up the rest of the segment selector registers
    movw    $KERNEL_DS, %cx
    movw    %cx, %ss
    movw    %cx, %ds
    movw    %cx, %es
    movw    %cx, %fs
    movw    %cx, %gs

    # Push the parameters that entry() expects (see kernel.c):
    # eax = multiboot magic
    # ebx = address of multiboot info struct
    pushl   %ebx
    pushl   %eax

    # Jump to the C entrypoint to the kernel.
    call    entry

    # We'll never get back here, but we put in a hlt anyway.
halt:
    hlt
    jmp     halt
