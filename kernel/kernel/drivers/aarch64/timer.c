/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <libkern/types.h>
#include <platform/aarch64/interrupts.h>
#include <platform/aarch64/registers.h>

static void aarch64_timer_write_reg(uint64_t val)
{
    asm volatile("msr cntp_ctl_el0, %x0"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static void aarch64_timer_write_ctrl(uint64_t val)
{
    asm volatile("msr cntp_tval_el0, %x0"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

void aarch64_timer_enable()
{
    aarch64_timer_write_reg(1);
}

void aarch64_timer_disable()
{
    aarch64_timer_write_reg(0b10);
}

void aarch64_timer_rearm()
{
    uint64_t el;
    asm volatile("mrs %x0, CNTFRQ_EL0"
                 : "=r"(el)
                 :);

    aarch64_timer_write_ctrl(el / 125);
}

void tick()
{
}

void aarch64_timer_init()
{
#ifdef TARGET_QEMU_VIRT
    irq_register_handler(30, 0, 0, tick, BOOT_CPU_MASK);
#endif

    aarch64_timer_disable();
    aarch64_timer_write_ctrl(0xfffffff);
    aarch64_timer_enable();
    aarch64_timer_rearm();
}
