/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_PLATFORM_H
#define _KERNEL_LIBKERN_PLATFORM_H

#include <libkern/c_attrs.h>

ALWAYS_INLINE int ctz32(unsigned int val)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(val);
#else
    for (u8 i = 0; i < 32; ++i) {
        if ((val >> i) & 1) {
            return i;
        }
    }
    return 0;
#endif
}

#endif // _KERNEL_LIBKERN_PLATFORM_H