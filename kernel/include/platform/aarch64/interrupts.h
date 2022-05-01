/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH64_INTERRUPTS_H
#define _KERNEL_PLATFORM_AARCH64_INTERRUPTS_H

#include <libkern/mask.h>
#include <libkern/types.h>

// TODO(aarch64): This is a duplicate with aarch32.

#define IRQ_HANDLERS_MAX 256
#define ALL_CPU_MASK 0xff
#define BOOT_CPU_MASK 0x01

typedef int irq_flags_t;
typedef int irq_line_t;
typedef uint8_t irq_priority_t;
typedef void (*irq_handler_t)();

// Currently flags maps to devtree irq_flags.
// Later we might need to enhance irq_flags_from_devtree() to use as translator.
#define IRQ_FLAG_EDGE_TRIGGERED (1 << 0)

struct gic_descritptor {
    uint32_t (*interrupt_descriptor)();
    void (*end_interrupt)(uint32_t int_desc);
    void (*enable_irq)(irq_line_t line, irq_priority_t prior, irq_flags_t type, int cpu_mask);
};
typedef struct gic_descritptor gic_descritptor_t;

void interrupts_setup();
void interrupts_setup_secondary_cpu();

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_flags_t flags, irq_handler_t func, int cpu_mask);
void irq_set_gic_desc(gic_descritptor_t gic_desc);

static inline irq_flags_t irq_flags_from_devtree(uint32_t devtree_irq_flags) { return (irq_flags_t)devtree_irq_flags; }

void gic_setup();
void gic_setup_secondary_cpu();

#endif /* _KERNEL_PLATFORM_AARCH64_INTERRUPTS_H */