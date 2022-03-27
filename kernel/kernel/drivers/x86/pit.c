/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/pit.h>
#include <libkern/kassert.h>
#include <libkern/log.h>
#include <platform/generic/system.h>
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
    port_byte_out(0x43, 0b110110);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
    system_enable_interrupts();
    return 0;
}

void pit_setup()
{
    int err = _pit_set_frequency(TIMER_TICKS_PER_SECOND);
    if (err) {
        kpanic("PIT: failed to set frequency");
    }
    set_irq_handler(IRQ0, pit_handler);
}

void pit_handler()
{
    system_disable_interrupts();
    cpu_tick();
    timeman_timer_tick();
    system_enable_interrupts();
    sched_tick();
}
