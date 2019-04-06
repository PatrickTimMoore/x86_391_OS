#include "exception_handler.h"
#include "system_call.h"

/*
 * exception_handler_0
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_0(){
 	printf("Divide Error Exception\n");
 	if(get_curr_pid() == -1){while(1);}
  else{halt(0);}

 }

 /*
 * exception_handler_1
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_1(){
 	printf("Debug Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_2
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_2(){
 	printf("NMI Interrupt\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_3
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_3(){
 	printf("Breakpoint Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_4
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_4(){
 	printf("Overflow Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_5
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_5(){
 	printf("BOUND Range Exceeded Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_6
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_6(){
 	printf("Invalid Opcode Exception");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_7
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_7(){
 	printf("Device Not Available Exception");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_8
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_8(){
 	printf("Double Fault Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{
    halt(0);
  }
 }

 /*
 * exception_handler_9
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_9(){
 	printf("Coprocessor Segment Overrun\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_10
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_10(){
 	printf("Invalid TSS Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_11
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_11(){
 	printf("Segment Not Present\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_12
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_12(){
 	printf("Stack Fault Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_13
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_13(){
 	printf("General Protection Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_14
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_14(){
 	printf("Page-Fault Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_15
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_15(){
 	printf("Intel Reserved\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_16
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_16(){
 	printf("x87 FPU Floating-Point Error\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_17
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_17(){
 	printf("Alignment Check Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_18
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_18(){
 	printf("Machine-Check Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_19
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_19(){
 	printf("SIMD Floating-Point Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_20
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
 void exception_handler_20(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_21
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_21(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_22
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_22(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_23
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_23(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_24
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_24(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

 /*
 * exception_handler_25
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/

  void exception_handler_25(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }


/*
 * exception_handler_26
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_26(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

/*
 * exception_handler_27
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_27(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

/*
 * exception_handler_28
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_28(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

/*
 * exception_handler_29
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_29(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

/*
 * exception_handler_30
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_30(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }

/*
 * exception_handler_31
 * input: none
 * output: none
 * now it just prints out the exception message
 * and jumps into while(1) loop
*/
  void exception_handler_31(){
 	printf("Undefined Exception\n");
  if(get_curr_pid() == -1){while(1);}
  else{halt(0);}
 }
