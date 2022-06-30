/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM64_REGISTERS_H
#define _KERNEL_PLATFORM_ARM64_REGISTERS_H

#include <libkern/types.h>
#include <platform/arm64/system.h>

extern uint64_t read_ip();
static inline uint64_t read_el()
{
    uint64_t el;
    asm volatile("mrs %x0, CurrentEL"
                 : "=r"(el)
                 :);
    return el >> 2;
}

static inline uint64_t read_sp()
{
    uint64_t sp;
    asm volatile("mov %x0, sp"
                 : "=r"(sp)
                 :);
    return sp;
}

static inline uint64_t read_fp()
{
    uint64_t fp;
    asm volatile("mov %x0, x29"
                 : "=r"(fp)
                 :);
    return fp;
}

static inline uint64_t read_cbar()
{
    uint32_t val;
    asm volatile("mrs %x0, S3_1_C15_C3_0"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint64_t read_cpacr()
{
    uint64_t cpacr;
    asm volatile("mrs %x0, CPACR_EL1"
                 : "=r"(cpacr)
                 :);
    return cpacr;
}

static inline void write_cpacr(uint64_t val)
{
    asm volatile("msr CPACR_EL1, %x0"
                 :
                 : "r"(val)
                 : "memory");
    asm volatile("isb");
}

static inline void write_tpidr(uint64_t val)
{
    asm volatile("msr TPIDR_EL1, %x0"
                 :
                 : "r"(val)
                 : "memory");
    asm volatile("isb");
}

#endif /* _KERNEL_PLATFORM_ARM64_REGISTERS_H */