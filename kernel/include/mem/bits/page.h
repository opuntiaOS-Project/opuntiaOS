/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_BITS_PAGE_H
#define _KERNEL_MEM_BITS_PAGE_H

enum PAGE_FLAGS {
    PAGE_WRITABLE = 0x1,
    PAGE_READABLE = 0x2,
    PAGE_EXECUTABLE = 0x4,
    PAGE_NOT_CACHEABLE = 0x8,
    PAGE_COW = 0x10,
    PAGE_USER = 0x20,
    PAGE_DEVICE = PAGE_READABLE | PAGE_WRITABLE | PAGE_NOT_CACHEABLE,
};

#endif // _KERNEL_MEM_BITS_PAGE_H