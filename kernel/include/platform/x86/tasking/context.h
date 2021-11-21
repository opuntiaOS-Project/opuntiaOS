/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H
#define _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

struct PACKED context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
};
typedef struct context context_t;

static inline uintptr_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->eip;
}

static inline void context_set_instruction_pointer(context_t* ctx, uintptr_t ip)
{
    ctx->eip = ip;
}

#endif // _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H