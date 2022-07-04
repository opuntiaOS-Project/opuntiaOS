/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
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
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rip;
};
typedef struct context context_t;

static inline uintptr_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->rip;
}

static inline void context_set_instruction_pointer(context_t* ctx, uintptr_t ip)
{
    ctx->rip = ip;
}

#endif // _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H