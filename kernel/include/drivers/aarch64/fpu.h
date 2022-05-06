/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH64_FPU_H
#define _KERNEL_DRIVERS_AARCH64_FPU_H

#include <libkern/kassert.h>
#include <libkern/types.h>
#include <platform/x86/registers.h>

#define FPU_STATE_ALIGNMENT (32)

typedef struct {
    uint64_t d[32];
} __attribute__((aligned(FPU_STATE_ALIGNMENT))) fpu_state_t;

void fpu_install();
void fpu_init_state(fpu_state_t* new_fpu_state);
extern void fpu_save(void*);
extern void fpu_restore(void*);

static inline void fpu_enable()
{
}

static inline void fpu_disable()
{
}

static inline int fpu_is_avail()
{
    return (((read_cpacr() >> 20) & 0b11) == 0b11);
}

static inline void fpu_make_avail()
{
    write_cpacr(read_cpacr() | ((0b11) << 20));
}

static inline void fpu_make_unavail()
{
    // Simply turn it off to make it unavailble.
    uint64_t val = read_cpacr() & (~((3ull) << 20));
    write_cpacr(val | ((0b01) << 20));
}

#endif //_KERNEL_DRIVERS_AARCH64_FPU_H
