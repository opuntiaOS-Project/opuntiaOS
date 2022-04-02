/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/sp804.h>
#include <drivers/devtree.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/aarch32/interrupts.h>
#include <tasking/cpu.h>
#include <tasking/sched.h>
#include <time/time_manager.h>

// #define DEBUG_SP804

static kmemzone_t mapped_zone;
volatile sp804_registers_t* timer1;

static inline uintptr_t _sp804_mmio_paddr()
{
    devtree_entry_t* device = devtree_find_device("sp804");
    if (!device) {
        kpanic("SP804: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline int _sp804_map_itself()
{
    uintptr_t mmio_paddr = _sp804_mmio_paddr();

    mapped_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    timer1 = (sp804_registers_t*)mapped_zone.ptr;
    return 0;
}

static inline void _sp804_clear_interrupt(volatile sp804_registers_t* timer)
{
    timer->intclr = 1;
}

static void _sp804_int_handler()
{
    system_disable_interrupts();
    log("   in sp804 [cpu %d]", THIS_CPU->id);
    _sp804_clear_interrupt(timer1);
    cpu_tick();
    timeman_timer_tick();
    system_enable_interrupts();
    sched_tick();
}

void sp804_install()
{
    _sp804_map_itself();
    timer1->load = SP804_CLK_HZ / TIMER_TICKS_PER_SECOND;
    timer1->control = SP804_ENABLE_MASK | SP804_PERIODIC_MASK | SP804_32_BIT_MASK | SP804_INTS_ENABLED_MASK;
    irq_register_handler(SP804_TIMER1_IRQ_LINE, 0, IRQ_TYPE_EDGE_TRIGGERED_MASK, _sp804_int_handler, ALL_CPU_MASK);
}

devman_register_driver_installation(sp804_install);