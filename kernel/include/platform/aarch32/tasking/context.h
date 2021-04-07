/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_TASKING_CONTEXT_H
#define _KERNEL_PLATFORM_AARCH32_TASKING_CONTEXT_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

typedef struct {
    uint32_t r[9];
    uint32_t lr;
} PACKED context_t;

static inline uint32_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->lr;
}

static inline void context_set_instruction_pointer(context_t* ctx, uint32_t ip)
{
    ctx->lr = ip;
}

#endif // _KERNEL_PLATFORM_AARCH32_TASKING_CONTEXT_H