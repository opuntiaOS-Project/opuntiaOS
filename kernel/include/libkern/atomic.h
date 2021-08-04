/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_ATOMIC_H
#define _KERNEL_LIBKERN_ATOMIC_H

#include <libkern/c_attrs.h>
#include <libkern/kassert.h>
#include <libkern/types.h>

static ALWAYS_INLINE int atomic_add_int(int* dst, int val)
{
    return __atomic_add_fetch(dst, val, __ATOMIC_SEQ_CST) + val;
}

static ALWAYS_INLINE uint32_t atomic_add_uint32(uint32_t* dst, uint32_t val)
{
    return __atomic_add_fetch(dst, val, __ATOMIC_SEQ_CST) + val;
}

static ALWAYS_INLINE void atomic_store_uint32(uint32_t* dst, uint32_t val)
{
    __atomic_store_n(dst, val, __ATOMIC_SEQ_CST);
}

static ALWAYS_INLINE uint32_t atomic_load_uint32(uint32_t* dst)
{
    return __atomic_load_n(dst, __ATOMIC_SEQ_CST);
}

#endif // _KERNEL_LIBKERN_LOCK_H