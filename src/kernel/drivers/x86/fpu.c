/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/x86/fpu.h>
#include <utils/mem.h>
#include <platform/x86/idt.h>
#include <log.h>

static fpu_state_t fpu_state;

void fpu_setup(void) {
    uint32_t tmp;
	asm volatile ("mov %%cr0, %0" : "=r"(tmp));
	tmp &= ~(1 << 2);
	tmp |= (1 << 1);
	asm volatile ("mov %0, %%cr0" :: "r"(tmp));

	asm volatile ("mov %%cr4, %0" : "=r"(tmp));
	tmp |= 3 << 9;
	asm volatile ("mov %0, %%cr4" :: "r"(tmp));

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

void fpu_reset_state(fpu_state_t* new_fpu_state)
{
    memcpy((uint8_t*)new_fpu_state, (uint8_t*)&fpu_state, sizeof(fpu_state_t));
}