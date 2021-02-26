#ifndef _KERNEL_PLATFORM_X86_IRQ_HANDLER_H
#define _KERNEL_PLATFORM_X86_IRQ_HANDLER_H

#include <platform/x86/idt.h>
#include <drivers/x86/display.h>

void irq_handler(trapframe_t *tf);
void irq_empty_handler();

#endif