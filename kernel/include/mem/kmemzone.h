/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_KMEMZONE_H
#define _KERNEL_MEM_KMEMZONE_H

#include <libkern/types.h>
#include <mem/bits/zone.h>

extern char __text_start[];
extern char __text_end[];
extern char __rodata_start[];
extern char __rodata_end[];
extern char __data_start[];
extern char __data_end[];
extern char __bss_start[];
extern char __bss_end[];
extern char __stack_start[];
extern char __stack_end[];
extern char __end[];

struct __kmemzone {
    union {
        uintptr_t start;
        uint8_t* ptr;
    };
    size_t len;
};
typedef struct __kmemzone kmemzone_t;

void kmemzone_init();
void kmemzone_init_stage2();

kmemzone_t kmemzone_new(size_t size);
kmemzone_t kmemzone_new_aligned(size_t size, size_t alignment);
int kmemzone_free(kmemzone_t zone);

#endif // _KERNEL_MEM_KMEMZONE_H