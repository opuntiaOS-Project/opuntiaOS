/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_TASKING_CONTEXT_H
#define _KERNEL_PLATFORM_RISCV64_TASKING_CONTEXT_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

typedef struct {
    uint64_t ra;
    uint64_t sp;
    uint64_t s[12];
} PACKED context_t;

static inline uintptr_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->ra;
}

static inline void context_set_instruction_pointer(context_t* ctx, uintptr_t ip)
{
    ctx->ra = ip;
}

#endif // _KERNEL_PLATFORM_RISCV64_TASKING_CONTEXT_H