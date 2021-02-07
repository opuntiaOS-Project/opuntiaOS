#ifndef __oneOS__INTERRUPTS__IRQHANDLER_H
#define __oneOS__INTERRUPTS__IRQHANDLER_H

#include <platform/x86/idt.h>
#include <drivers/x86/display.h>

void irq_handler(trapframe_t *tf);
void irq_empty_handler();

#endif