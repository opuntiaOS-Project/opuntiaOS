#ifndef __oneOS__INTERRUPTS__IRQHANDLER_H
#define __oneOS__INTERRUPTS__IRQHANDLER_H

#include <idt_common_data.h>

void irq_handler_master(regs_t regs);
void irq_handler_slave(regs_t regs);
void irq_handler_redirect(regs_t regs);

void irq_handler_null();

#endif