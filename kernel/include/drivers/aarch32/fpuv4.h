/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH32_FPUV4_H
#define _KERNEL_DRIVERS_AARCH32_FPUV4_H

#include <drivers/driver_manager.h>
#include <libkern/mask.h>
#include <libkern/types.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/registers.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>

typedef struct {
    uint64_t d[32];
} fpu_state_t;

void fpuv4_install();
void fpu_init_state(fpu_state_t* new_fpu_state);
extern uint32_t read_fpexc();
extern void write_fpexc(uint32_t);
extern void fpu_save(void*);
extern void fpu_restore(void*);

static inline void fpu_enable()
{
    write_fpexc(read_fpexc() | (1 << 30));
}

static inline void fpu_disable()
{
    write_fpexc(read_fpexc() & (~(1 << 30)));
}

static inline int fpu_is_avail()
{
    return (((read_cpacr() >> 20) & 0b1111) == 0b1111);
}

static inline void fpu_make_avail()
{
    write_cpacr(read_cpacr() | ((0b1111) << 20));
}

static inline void fpu_make_unavail()
{
    // Simply turn it off to make it unavailble.
    uint32_t val = read_cpacr() & (~((0b1111) << 20));
    write_cpacr(val | ((0b0101) << 20));
}

#endif //_KERNEL_DRIVERS_AARCH32_FPUV4_H
