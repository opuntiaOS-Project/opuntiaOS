/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_REGISTERS_H
#define _KERNEL_PLATFORM_X86_REGISTERS_H

#include <libkern/types.h>

extern uintptr_t read_ip();
static inline uintptr_t read_cr2()
{
    uintptr_t val;
    asm volatile("movl %%cr2,%0"
                 : "=r"(val));
    return val;
}

static inline uintptr_t read_cr3()
{
    uintptr_t val;
    asm volatile("movl %%cr3,%0"
                 : "=r"(val));
    return val;
}

static inline uintptr_t read_sp()
{
    uintptr_t val;
    asm volatile("movl %%esp,%0"
                 : "=r"(val));
    return val;
}

static inline uintptr_t read_bp()
{
    uintptr_t val;
    asm volatile("movl %%ebp,%0"
                 : "=r"(val));
    return val;
}

static inline uintptr_t read_cr0()
{
    uintptr_t val;
    asm volatile("movl %%cr0, %0"
                 : "=r"(val));
    return val;
}

static inline void write_cr0(uintptr_t val)
{
    asm volatile("movl %0, %%cr0"
                 :
                 : "r"(val));
}

#endif /* _KERNEL_PLATFORM_X86_REGISTERS_H */