/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define MAX_MASTER    7
#define MIN_MASTER    0
#define MAX_SLAVE     15
#define MIN_SLAVE     8

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
	//clear the interrupts 
	cli();
	//mask all interrupts on the PIC
	outb(MASK_ALL, MASTER_8259_PORT_2);
	outb(MASK_ALL, SLAVE_8259_PORT_2);
	//initialize the master PIC
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT_2);
	outb(ICW3_MASTER, MASTER_8259_PORT_2);
	outb(ICW4, MASTER_8259_PORT_2);
	//initialize the slave PIC
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT_2);
	outb(ICW3_SLAVE, SLAVE_8259_PORT_2);
	outb(ICW4, SLAVE_8259_PORT_2);

	outb(MASK_ALL, MASTER_8259_PORT_2);
	outb(MASK_ALL, SLAVE_8259_PORT_2);
    //enable the slave PIC
    enable_irq(SLAVE_LINE);
    //restore the interrupts
	sti();
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	uint32_t old_mask;
	uint32_t new_mask;
	//check if the request is out of range
	if(irq_num < MIN_MASTER || irq_num > MAX_SLAVE ){
        return;
	}
	//check if it is a master or a slave pic
	if(irq_num <= MAX_MASTER){
        //read the old mask from mask register
        old_mask= inb(MASTER_8259_PORT_2);
        //set the new mask
        new_mask= ~(1 << irq_num);
        new_mask= new_mask & old_mask;
        //apply the new mask to the pic
        outb(new_mask, MASTER_8259_PORT_2);
	}
	else{
		//we need mask slave pic
		//read the old mask from mask register
        old_mask= inb(SLAVE_8259_PORT_2);
        //set the new mask
        new_mask= ~(1 << (irq_num - MIN_SLAVE));
        new_mask= new_mask & old_mask;
        //apply the new mask to the pic
        outb(new_mask, SLAVE_8259_PORT_2);
	}
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint32_t old_mask;
	uint32_t new_mask;
	//check if the request is out of range
	if(irq_num < MIN_MASTER || irq_num > MAX_SLAVE ){
        return;
	}
	//check if it is a master or a slave pic
	if(irq_num <= MAX_MASTER){
        //read the old mask from mask register
        old_mask= inb(MASTER_8259_PORT_2);
        //set the new mask
        new_mask= (1 << irq_num);
        new_mask= new_mask | old_mask;
        //apply the new mask to the pic
        outb(new_mask, MASTER_8259_PORT_2);
	}
	else{
		//we need mask slave pic
		//read the old mask from mask register
        old_mask= inb(SLAVE_8259_PORT_2);
        //set the new mask
        new_mask= (1 << (irq_num - MIN_SLAVE));
        new_mask= new_mask | old_mask;
        //apply the new mask to the pic
        outb(new_mask, SLAVE_8259_PORT_2);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	uint32_t eoi;
	if(irq_num < MIN_MASTER || irq_num > MAX_SLAVE ){
        return;
	}
	if(irq_num <= MAX_MASTER){
        //calculate the eoi
        eoi= EOI | irq_num;
        //send the eoi to the pic
        outb(eoi, MASTER_8259_PORT);
	}
	else{
		//calculate the eoi
        eoi= EOI | (irq_num- MIN_SLAVE);
        //send the eoi to the slave pic
        outb(eoi, SLAVE_8259_PORT);
        //also send eoi to the line 2 on master
        outb(EOI + SLAVE_LINE, MASTER_8259_PORT);
    }

}
