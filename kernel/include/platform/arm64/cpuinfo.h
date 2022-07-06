/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM64_CPUINFO_H
#define _KERNEL_PLATFORM_ARM64_CPUINFO_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>
#include <platform/generic/cpu.h>

static inline bool cpuinfo_has_1gb_pages()
{
    return true;
}

#endif // _KERNEL_PLATFORM_ARM64_CPUINFO_H
