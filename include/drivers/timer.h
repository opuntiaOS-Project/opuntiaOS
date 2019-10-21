#ifndef __oneOS__HARDWARECOMMUNICATION__TIMER_H
#define __oneOS__HARDWARECOMMUNICATION__TIMER_H

#include <types.h>
#include <x86/idt.h>

void init_timer();
void set_frequency(uint16_t freq);
void timer_handler();

#endif

