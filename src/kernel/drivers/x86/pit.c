/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/x86/pit.h>
#include <tasking/sched.h>
#include <time/time_manager.h>
#include <utils/kassert.h>
#include <platform/x86/registers.h>
#include <log.h>

static int ticks_to_sched = 0;
static int second = 1000;
static int _pit_set_frequency(uint16_t freq);

static int _pit_set_frequency(uint16_t freq)
{
    disable_intrs();
    uint32_t divisor = PIT_BASE_FREQ / freq;
    if (divisor > 0xffff) {
        return -1;
    }
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    port_byte_out(0x43, 0x36); // 0b110110
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
    enable_intrs();
    return 0;
}

void pit_setup()
{
    RUNNIG_THREAD = 0; // TODO: Remove it from here 
    ticks_to_sched = SCHED_INT;
    int res = _pit_set_frequency(PIT_TICKS_PER_SECOND);
    if (res < 0) {
        kpanic("Pit: failed to set freq");
    }
    set_irq_handler(IRQ0, pit_handler);
}

void pit_handler()
{
    timeman_pit_tick();
    if (RUNNIG_THREAD) {
        ticks_to_sched--;
        second--;
        if (second < 0) {
            second = 1000;
            log("----- second -----");
        }
        if (ticks_to_sched < 0) {
            ticks_to_sched = SCHED_INT;
            resched();
        }
    }
}
