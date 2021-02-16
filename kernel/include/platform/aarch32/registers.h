/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__AARCH32__REGS_H
#define __oneOS__AARCH32__REGS_H

#include <types.h>

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


#endif /* __oneOS__AARCH32__REGS_H */