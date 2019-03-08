#include "exception_handler.h"


 void exception_handler_0(){
 	printf("Divide Error Exception\n");
 	while(1);
 }
 void exception_handler_1(){
 	printf("Debug Exception\n");
 }
 void exception_handler_2(){
 	printf("NMI Interrupt\n");
 }
 void exception_handler_3(){
 	printf("Breakpoint Exception\n");
 }
 void exception_handler_4(){
 	printf("Overflow Exception\n");
 }
 void exception_handler_5(){
 	printf("BOUND Range Exceeded Exception\n");
 }
 void exception_handler_6(){
 	printf("Invalid Opcode Exception");
 }
 void exception_handler_7(){
 	printf("Device Not Available Exception");
 }
 void exception_handler_8(){
 	printf("Double Fault Exception\n");
 }
 void exception_handler_9(){
 	printf("Coprocessor Segment Overrun\n");
 }
 void exception_handler_10(){
 	printf("Invalid TSS Exception\n");
 }
 void exception_handler_11(){
 	printf("Segment Not Present\n");
 }
 void exception_handler_12(){
 	printf("Stack Fault Exception\n");
 }
 void exception_handler_13(){
 	printf("General Protection Exception\n");
 }
 void exception_handler_14(){
 	printf("Page-Fault Exception\n");
 }
 void exception_handler_15(){
 	printf("Intel Reserved\n");
 }
 void exception_handler_16(){
 	printf("x87 FPU Floating-Point Error\n");
 }
 void exception_handler_17(){
 	printf("Alignment Check Exception\n");
 }
 void exception_handler_18(){
 	printf("Machine-Check Exception\n");
 }

 void exception_handler_19(){
 	printf("SIMD Floating-Point Exception\n");
 }

