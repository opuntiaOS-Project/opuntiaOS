#ifndef _KERNEL_PLATFORM_X86_ISR_HANDLER_H
#define _KERNEL_PLATFORM_X86_ISR_HANDLER_H

#include <libkern/types.h>
#include <platform/x86/idt.h>
#include <drivers/x86/display.h>

void isr_handler(trapframe_t *tf);
void isr_standart_handler(trapframe_t *tf);
uint32_t rcr2();

#endif
