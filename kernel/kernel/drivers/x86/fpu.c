/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/fpu.h>
#include <libkern/log.h>
#include <libkern/mem.h>
#include <platform/x86/idt.h>

static fpu_state_t fpu_state;

void fpu_setup(void)
{
    uint32_t tmp;
    asm volatile("mov %%cr0, %0"
                 : "=r"(tmp));
    tmp &= ~(1 << 2);
    tmp |= (1 << 1);
    asm volatile("mov %0, %%cr0" ::"r"(tmp));

    asm volatile("mov %%cr4, %0"
                 : "=r"(tmp));
    tmp |= 3 << 9;
    asm volatile("mov %0, %%cr4" ::"r"(tmp));
}

void fpu_handler()
{
    asm volatile("clts");
}

void fpu_init()
{
    fpu_setup();
    asm volatile("fninit");
    asm volatile("fxsave %0"
                 : "=m"(fpu_state));
    set_irq_handler(IRQ7, fpu_handler);
}

void fpu_init_state(fpu_state_t* new_fpu_state)
{
    memcpy(new_fpu_state, &fpu_state, sizeof(fpu_state_t));
}