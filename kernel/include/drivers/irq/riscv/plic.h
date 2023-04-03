/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_IRQ_RISCV_PLIC_H
#define _KERNEL_DRIVERS_IRQ_RISCV_PLIC_H

#include <drivers/driver_manager.h>
#include <drivers/irq/irq_api.h>

void plic_install();
void plic_enable_irq(irq_line_t id, irq_priority_t prior, irq_flags_t flags, int cpu_mask);
uint32_t plic_interrupt_descriptor();
void plic_end(uint32_t id);

#endif //_KERNEL_DRIVERS_IRQ_RISCV_PLIC_H