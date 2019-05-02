#ifndef SIGNAL_H
#define SIGNAL_H

#include "types.h"

#define DIV_ZERO	0
#define SEGFAULT	1	//God we missed you
#define INTERRUPT	2
#define ALARM		3
#define USER1		4

extern int signals_ready;

void kill_task(void);
void ignore(void);


// void init_signals(void);
void sig_pending(void);
int32_t raise_sig(uint32_t signum);

#endif

