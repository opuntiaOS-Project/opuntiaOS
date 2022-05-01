/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH64_REGISTERS_H
#define _KERNEL_PLATFORM_AARCH64_REGISTERS_H

#include <libkern/types.h>
#include <platform/aarch64/system.h>

static inline uint64_t current_el()
{
    uint64_t el;
    asm volatile("mrs %x0, CurrentEL"
                 : "=r"(el)
                 :);
    return el >> 2;
}

#endif /* _KERNEL_PLATFORM_AARCH64_REGISTERS_H */