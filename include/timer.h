#ifndef __oneOS__HARDWARECOMMUNICATION__TIMER_H
#define __oneOS__HARDWARECOMMUNICATION__TIMER_H

#include <types.h>

void init_timer();
void set_frequency(u_int16 freq);
void timer_handler();

#endif

