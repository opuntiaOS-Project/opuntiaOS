/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_IRQ_IRQ_API_H
#define _KERNEL_DRIVERS_IRQ_IRQ_API_H

#include <libkern/types.h>

// This value shows the maximum number of irqs OS supports. It is irqdev independent,
// thus irqdev implementations should double check the irqlines.
#define IRQ_HANDLERS_MAX 256

#define ALL_CPU_MASK 0xff
#define BOOT_CPU_MASK 0x01

typedef int irq_flags_t;
typedef int irq_line_t;
typedef uint8_t irq_priority_t;
typedef void (*irq_handler_t)(irq_line_t line);

// Currently flags maps to devtree irq_flags.
// Later we might need to enhance irq_flags_from_devtree() to use as translator.
#define IRQ_FLAG_EDGE_TRIGGERED (1 << 0)

struct irqdev_descritptor {
    uint32_t (*interrupt_descriptor)();
    void (*end_interrupt)(uint32_t int_desc);
    void (*enable_irq)(irq_line_t line, irq_priority_t prior, irq_flags_t type, int cpu_mask);
};
typedef struct irqdev_descritptor irqdev_descritptor_t;

static inline irq_flags_t irq_flags_from_devtree(uint32_t devtree_irq_flags) { return (irq_flags_t)devtree_irq_flags; }

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_flags_t flags, irq_handler_t func, int cpu_mask);
void irq_set_dev(irqdev_descritptor_t irqdev_desc);
irq_line_t irqline_from_id(int id);

#endif