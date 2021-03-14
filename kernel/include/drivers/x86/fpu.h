/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_FPU_H
#define _KERNEL_DRIVERS_X86_FPU_H

#include <libkern/types.h>

typedef struct {
    uint8_t buffer[512];
} __attribute__((aligned(16))) fpu_state_t;

void fpu_handler();
void fpu_init();
void fpu_init_state(fpu_state_t* new_fpu_state);

static inline void fpu_save(fpu_state_t* fpu_state)
{
    asm volatile("fxsave %0"
                 : "=m"(*fpu_state));
}

static inline void fpu_restore(fpu_state_t* fpu_state)
{
    asm volatile("fxrstor %0"
                 :
                 : "m"(*fpu_state));
}

#endif //_KERNEL_DRIVERS_X86_FPU_H
