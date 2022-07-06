/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_CPUINFO_H
#define _KERNEL_PLATFORM_X86_CPUINFO_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>
#include <platform/generic/cpu.h>

enum CPUFEAT_FLAGS {
    CPUFEAT_FPU = (1 << 0),
    CPUFEAT_PSE = (1 << 1),
    CPUFEAT_PAE = (1 << 2),
    CPUFEAT_CLFSH = (1 << 3),
    CPUFEAT_SSE = (1 << 4),
    CPUFEAT_SSE2 = (1 << 5),
    CPUFEAT_SSE3 = (1 << 6),
    CPUFEAT_SSSE3 = (1 << 7),
    CPUFEAT_SSE4_1 = (1 << 8),
    CPUFEAT_SSE4_2 = (1 << 9),
    CPUFEAT_XSAVE = (1 << 10),
    CPUFEAT_AVX = (1 << 11),
    CPUFEAT_PDPE1GB = (1 << 12),
};

void cpuinfo_init();

static inline bool cpuinfo_has_1gb_pages()
{
    return TEST_FLAG(THIS_CPU->cpufeat, CPUFEAT_PDPE1GB);
}

#endif // _KERNEL_PLATFORM_X86_CPUINFO_H
