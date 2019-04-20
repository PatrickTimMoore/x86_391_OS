#ifndef SIGNAL_H
#define SIGNAL_H

#include "types.h"

int sig_bitmap[5];

void kill_task();
void ignore();

void signal_pending();

#endif