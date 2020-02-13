#ifndef __oneOS__INTERRUPTS__ISRHANDLER_H
#define __oneOS__INTERRUPTS__ISRHANDLER_H

#include <types.h>
#include <x86/idt.h>
#include <drivers/display.h>

void isr_handler(trapframe_t *tf);
void isr_standart_handler(trapframe_t *tf);
uint32_t rcr2();

#endif
