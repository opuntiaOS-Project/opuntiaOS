/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_VMM_PTE_H
#define _KERNEL_PLATFORM_AARCH32_VMM_PTE_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>
#include <mem/bits/mmu.h>

typedef uint32_t ptable_entity_t;

struct PACKED page_desc {
    union {
        struct {
            unsigned int xn : 1; // Execute never. Stops execution of page.
            unsigned int one : 1; // Always one for tables
            unsigned int b : 1; // cacheable
            unsigned int c : 1; // Cacheable
            unsigned int ap1 : 2;
            unsigned int tex : 3;
            unsigned int ap2 : 1;
            unsigned int s : 1;
            unsigned int ng : 1;
            unsigned int baddr : 20;
        };
        uint32_t data;
    };
};
typedef struct page_desc page_desc_t;

#define PAGE_DESC_FRAME_OFFSET 12

#endif //_KERNEL_PLATFORM_AARCH32_VMM_PTE_H
