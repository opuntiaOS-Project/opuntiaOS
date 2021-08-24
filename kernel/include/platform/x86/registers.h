/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_REGISTERS_H
#define _KERNEL_PLATFORM_X86_REGISTERS_H

#include <libkern/types.h>

uint32_t read_cr2();
uint32_t read_cr3();
uint32_t read_esp();
extern uint32_t read_eip();

static inline uint32_t read_ebp()
{
    uint32_t val;
    asm volatile("movl %%ebp,%0"
                 : "=r"(val));
    return val;
}

static inline uint32_t read_cr0()
{
    uint32_t val;
    asm volatile("movl %%cr0, %0"
                 : "=r"(val));
    return val;
}

static inline void write_cr0(uint32_t val)
{
    asm volatile("movl %0, %%cr0"
                 :
                 : "r"(val));
}

#endif /* _KERNEL_PLATFORM_X86_REGISTERS_H */