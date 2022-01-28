/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_BITS_MMU_H
#define _KERNEL_MEM_BITS_MMU_H

#include <libkern/types.h>

enum MMU_FLAGS {
    MMU_FLAG_PERM_WRITE = (1 << 0),
    MMU_FLAG_PERM_READ = (1 << 1),
    MMU_FLAG_PERM_EXEC = (1 << 2),
    MMU_FLAG_UNCACHED = (1 << 3),
    MMU_FLAG_NONPRIV = (1 << 4),
    MMU_FLAG_INVALID = (1 << 5),
    MMU_FLAG_COW = (1 << 6), // TODO: Remove this flag.
    MMU_FLAG_DEVICE = MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED,
};
typedef uint32_t mmu_flags_t;

#endif // _KERNEL_MEM_BITS_MMU_H