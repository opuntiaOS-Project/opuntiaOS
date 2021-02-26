/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/sp804.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <platform/aarch32/interrupts.h>
#include <tasking/sched.h>
#include <time/time_manager.h>

// #define DEBUG_SP804

static zone_t mapped_zone;
volatile sp804_registers_t* timer1 = (sp804_registers_t*)SP804_TIMER1_BASE;

static inline int _sp804_map_itself()
{
    mapped_zone = zoner_new_zone(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, SP804_TIMER1_BASE, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
    timer1 = (sp804_registers_t*)mapped_zone.ptr;
    return 0;
}

static inline void _sp804_clear_interrupt(volatile sp804_registers_t* timer)
{
    timer->intclr = 1;
}

static void _sp804_int_handler()
{
    _sp804_clear_interrupt(timer1);
    timeman_timer_tick();
    if (RUNNIG_THREAD) {
        resched();
    }
}

void sp804_install()
{
    _sp804_map_itself();
    timer1->load = SP804_CLK_HZ / 125;
    timer1->control = SP804_ENABLE_MASK | SP804_PERIODIC_MASK | SP804_32_BIT_MASK | SP804_INTS_ENABLED_MASK;
    irq_register_handler(SP804_TIMER1_IRQ_LINE, 0, IRQ_TYPE_EDGE_TRIGGERED_MASK, _sp804_int_handler);
}