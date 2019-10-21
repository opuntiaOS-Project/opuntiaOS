#ifndef __oneOS__INTERRUPTS__IRQHANDLER_H
#define __oneOS__INTERRUPTS__IRQHANDLER_H

#include <x86/idt.h>
#include <drivers/display.h>

uint32_t irq_handler(uint8_t int_no, uint32_t esp);
void irq_redirect(uint8_t int_no);
void irq_empty_handler();

#endif