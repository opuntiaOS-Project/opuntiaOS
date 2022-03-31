/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_CPUID_H
#define _KERNEL_PLATFORM_X86_CPUID_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

struct cpuid {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};
typedef struct cpuid cpuid_t;

cpuid_t get_cpuid(int eax, int ecx);

#endif // _KERNEL_PLATFORM_X86_CPUID_H
