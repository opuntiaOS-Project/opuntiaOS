/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/irq/irq_api.h>
#include <drivers/timer/arm/sp804.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/cpu.h>
#include <tasking/sched.h>
#include <time/time_manager.h>

// #define DEBUG_SP804

static kmemzone_t mapped_zone;
volatile sp804_registers_t* timer1;

static inline uintptr_t _sp804_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("SP804: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline int _sp804_map_itself(device_t* dev)
{
    uintptr_t mmio_paddr = _sp804_mmio_paddr(dev->device_desc.devtree.entry);

    mapped_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    timer1 = (sp804_registers_t*)mapped_zone.ptr;
    return 0;
}

static inline void _sp804_clear_interrupt(volatile sp804_registers_t* timer)
{
    timer->intclr = 1;
}

static void _sp804_int_handler(irq_line_t il)
{
    _sp804_clear_interrupt(timer1);
    cpu_tick();
    timeman_timer_tick();
    sched_tick();
}

int sp804_init(device_t* dev)
{
    if (_sp804_map_itself(dev)) {
#ifdef DEBUG_SP804
        log_error("SP804: Can't map itself!");
#endif
        return -1;
    }

    timer1->load = SP804_CLK_HZ / TIMER_TICKS_PER_SECOND;
    timer1->control = SP804_ENABLE_MASK | SP804_PERIODIC_MASK | SP804_32_BIT_MASK | SP804_INTS_ENABLED_MASK;

    devtree_entry_t* devtree_entry = dev->device_desc.devtree.entry;
    ASSERT(devtree_entry->irq_lane > 0);
    irq_flags_t irqflags = irq_flags_from_devtree(devtree_entry->irq_flags);
    irq_register_handler(devtree_entry->irq_lane, devtree_entry->irq_priority, irqflags, _sp804_int_handler, ALL_CPU_MASK);
    return 0;
}

static driver_desc_t _sp804_keyboard_driver_info()
{
    driver_desc_t ms_desc = { 0 };
    ms_desc.type = DRIVER_TIMER;
    ms_desc.system_funcs.init_with_dev = sp804_init;
    ms_desc.system_funcs.recieve_notification = NULL;
    return ms_desc;
}

void sp804_install()
{
    devman_register_driver(_sp804_keyboard_driver_info(), "sp804");
}

devman_register_driver_installation(sp804_install);