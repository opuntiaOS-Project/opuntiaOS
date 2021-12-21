/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_KMEMZONE_H
#define _KERNEL_MEM_KMEMZONE_H

#include <libkern/types.h>
#include <mem/bits/zone.h>

struct __kmemzone {
    union {
        uintptr_t start;
        uint8_t* ptr;
    };
    size_t len;
};
typedef struct __kmemzone kmemzone_t;

void kmemzone_init(uint32_t start_vaddr);
void kmemzone_init_stage2();

kmemzone_t kmemzone_new(uint32_t size);
kmemzone_t kmemzone_new_aligned(uint32_t size, uint32_t alignment);
int kmemzone_free(kmemzone_t zone);

#endif // _KERNEL_MEM_KMEMZONE_H