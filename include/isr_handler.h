#ifndef __oneOS__INTERRUPTS__ISRHANDLER_H
#define __oneOS__INTERRUPTS__ISRHANDLER_H

#include <types.h>
#include <drivers/display.h>

uint32_t isr_handler(uint8_t int_no, uint32_t esp);
void isr_standart_handler(uint8_t int_no);

#endif