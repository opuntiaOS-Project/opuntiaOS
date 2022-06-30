/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM32_REGISTERS_H
#define _KERNEL_PLATFORM_ARM32_REGISTERS_H

#include <libkern/types.h>
#include <platform/arm32/system.h>

static inline uint32_t extract_bits(uint32_t a, int bottom, int top)
{
    int diff = top - bottom + 1;
    return (a >> bottom) & ((1 << (diff)) - 1);
}

static inline uint32_t read_r3()
{
    uint32_t val;
    asm volatile("mov %0, r3"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint32_t read_far()
{
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c6, c0, 0"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint32_t read_cbar()
{
    uint32_t val;
    asm volatile("mrc p15, 4, %0, c15, c0, 0"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint32_t read_ifsr()
{
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c5, c0, 1"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint32_t read_dfsr()
{
    uint32_t val;
    asm volatile("mrc p15, 0, %0, c5, c0, 0"
                 : "=r"(val)
                 :);
    return val;
}

static inline uint32_t read_cpsr()
{
    uint32_t cpsr;
    asm volatile("mrs %0, cpsr"
                 : "=r"(cpsr)
                 :);
    return cpsr;
}

static inline uint32_t read_spsr()
{
    uint32_t spsr;
    asm volatile("mrs %0, spsr"
                 : "=r"(spsr)
                 :);
    return spsr;
}

static inline uint32_t read_cpacr()
{
    uint32_t cpacr;
    asm volatile("mrc p15, 0, %0, c1, c0, 2"
                 : "=r"(cpacr)
                 :);
    return cpacr;
}

static inline void write_cpacr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 2"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static inline uint32_t read_nsacr()
{
    uint32_t cpacr;
    asm volatile("mrc p15, 0, %0, c1, c1, 2"
                 : "=r"(cpacr)
                 :);
    return cpacr;
}

static inline void write_nsacr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c1, c1, 2"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static inline uint32_t read_hcptr()
{
    uint32_t cpacr;
    asm volatile("mrc p15, 4, %0, c1, c1, 2"
                 : "=r"(cpacr)
                 :);
    return cpacr;
}

static inline void write_hcptr(uint32_t val)
{
    asm volatile("mcr p15, 4, %0, c1, c1, 2"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static inline uint32_t read_cpu_id_register()
{
    uint32_t res;
    asm volatile("mrc p15, 0, %0, c0, c0, 5"
                 : "=r"(res)
                 :);
    return res;
}

static inline uint32_t read_midr()
{
    uint32_t res;
    asm volatile("mrc p15, 0, %0, c0, c0, 0"
                 : "=r"(res)
                 :);
    return res;
}

static inline uint32_t read_actlr()
{
    uint32_t res;
    asm volatile("mrc p15, 0, %0, c1, c0, 1"
                 : "=r"(res)
                 :);
    return res;
}

static inline void write_actlr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 1"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static inline void write_iciallu(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c7, c5, 0"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

#endif /* _KERNEL_PLATFORM_ARM32_REGISTERS_H */