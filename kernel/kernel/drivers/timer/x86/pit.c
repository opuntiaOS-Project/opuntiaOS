/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/irq/irq_api.h>
#include <drivers/timer/x86/pit.h>
#include <libkern/kassert.h>
#include <libkern/log.h>
#include <platform/x86/port.h>
#include <platform/x86/system.h>
#include <tasking/cpu.h>
#include <tasking/sched.h>
#include <time/time_manager.h>

static int ticks_to_sched = 0;
static int second = TIMER_TICKS_PER_SECOND;
static int _pit_set_frequency(uint16_t freq);

static int _pit_set_frequency(uint16_t freq)
{
    system_disable_interrupts();
    uint32_t divisor = PIT_BASE_FREQ / freq;
    if (divisor > 0xffff) {
        return -1;
    }
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    port_write8(0x43, 0b110110);
    port_write8(0x40, low);
    port_write8(0x40, high);
    system_enable_interrupts();
    return 0;
}

void pit_handler(irq_line_t il)
{
    system_disable_interrupts();
    cpu_tick();
    timeman_timer_tick();
    system_enable_interrupts();
    sched_tick();
}

void pit_setup()
{
    int err = _pit_set_frequency(TIMER_TICKS_PER_SECOND);
    if (err) {
        kpanic("PIT: failed to set frequency");
    }
    irq_register_handler(irqline_from_id(0), 0, 0, pit_handler, BOOT_CPU_MASK);
}
