/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_REGISTERS_H
#define _KERNEL_PLATFORM_RISCV64_REGISTERS_H

#include <libkern/types.h>
#include <platform/riscv64/system.h>

extern uint64_t read_ip();
static inline uint64_t read_scause()
{
    uint64_t x;
    asm volatile("csrr %0, scause"
                 : "=r"(x));
    return x;
}

static inline uint64_t read_stval()
{
    uint64_t x;
    asm volatile("csrr %0, stval"
                 : "=r"(x));
    return x;
}

static inline uint64_t read_sip()
{
    uint64_t x;
    asm volatile("csrr %0, sip"
                 : "=r"(x));
    return x;
}

static inline void write_sip(uint64_t val)
{
    asm volatile("csrw sip, %0"
                 :
                 : "r"(val)
                 : "memory");
}

#endif /* _KERNEL_PLATFORM_RISCV64_REGISTERS_H */