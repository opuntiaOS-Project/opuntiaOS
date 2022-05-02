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
    // TODO(aarch64): fix
    uint64_t d[32];
} __attribute__((aligned(FPU_STATE_ALIGNMENT))) fpu_state_t;

void fpu_handler();
void fpu_init();
void fpu_init_state(fpu_state_t* new_fpu_state);

static inline void fpu_save(fpu_state_t* fpu_state)
{
    ASSERT(false);
}

static inline void fpu_restore(fpu_state_t* fpu_state)
{
    ASSERT(false);
}

static inline int fpu_is_avail()
{
    return false;
}

static inline void fpu_make_avail()
{
    ASSERT(false);
}

static inline void fpu_make_unavail()
{
    ASSERT(false);
}

#endif //_KERNEL_DRIVERS_AARCH64_FPU_H
