/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch64/gicv2.h>
#include <drivers/aarch64/timer.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/aarch64/interrupts.h>
#include <platform/aarch64/registers.h>
#include <platform/aarch64/system.h>
#include <platform/aarch64/tasking/trapframe.h>

#define ERR_BUF_SIZE 64
static char err_buf[ERR_BUF_SIZE];

static gic_descritptor_t gic_descriptor;
static irq_handler_t _irq_handlers[IRQ_HANDLERS_MAX];

static void _irq_empty_handler()
{
    return;
}

static void init_irq_handlers()
{
    for (int i = 0; i < IRQ_HANDLERS_MAX; i++) {
        _irq_handlers[i] = _irq_empty_handler;
    }
}

void interrupts_setup()
{
    system_disable_interrupts();
    system_enable_interrupts_only_counter(); // Reset counter
    extern void set_up_vector_table();
    set_up_vector_table();
    init_irq_handlers();
}

void serror_handler(trapframe_t* tf)
{
    log("serror_handler");
}

void sync_handler(trapframe_t* tf)
{
    log("sync_handler");
}

void irq_handler(trapframe_t* tf)
{
    // Qemu_virt_here target got it here
    log("irq_handler");
    aarch64_timer_rearm();
    gic_descriptor.end_interrupt(30);
}

void fast_irq_handler()
{
    // Apl target got it here
    log("fast_irq_handler");
    aarch64_timer_rearm();
}

void gic_setup()
{
    gicv2_install();
}

void gic_setup_secondary_cpu()
{
    gicv2_install_secondary_cpu();
}

void irq_set_gic_desc(gic_descritptor_t gic_desc)
{
    gic_descriptor = gic_desc;
}

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_flags_t flags, irq_handler_t func, int cpu_mask)
{
    _irq_handlers[line] = func;
    gic_descriptor.enable_irq(line, prior, flags, cpu_mask);
}