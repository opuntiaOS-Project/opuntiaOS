/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_INTERRUPTS_H
#define _KERNEL_PLATFORM_AARCH32_INTERRUPTS_H

#include <drivers/irq/irq_api.h>
#include <libkern/mask.h>
#include <libkern/types.h>

void interrupts_setup();
void interrupts_setup_secondary_cpu();

extern char STACK_ABORT_TOP;
extern char STACK_UNDEFINED_TOP;
extern char STACK_IRQ_TOP;
extern char STACK_SVC_TOP;
extern char STACK_TOP;

extern void swi(uint32_t num);
extern void set_svc_stack(uint32_t stack);
extern void set_irq_stack(uint32_t stack);
extern void set_abort_stack(uint32_t stack);
extern void set_undefined_stack(uint32_t stack);

extern void reset_handler();
extern void undefined_handler();
extern void svc_handler();
extern void prefetch_abort_handler();
extern void data_abort_handler();
extern void irq_handler();
extern void fast_irq_handler();

void gic_setup();
void gic_setup_secondary_cpu();

#endif /* _KERNEL_PLATFORM_AARCH32_INTERRUPTS_H */