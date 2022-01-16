/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_VMM_VMM_H
#define _BOOT_VMM_VMM_H

#include "consts.h"
#include <libboot/types.h>

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

struct PACKED table_desc {
    union {
        struct {
            int valid : 1; /* Valid mapping */
            int zero1 : 1;
            int zero2 : 1;
            int ns : 1;
            int zero3 : 1;
            int domain : 4;
            int imp : 1;
            int baddr : 22;
        };
        uint32_t data;
    };
};
typedef struct table_desc table_desc_t;

typedef struct {
    page_desc_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    table_desc_t entities[VMM_PDE_COUNT];
} pdirectory_t;

void vm_setup();
void vm_setup_secondary_cpu();

#endif // _BOOT_VMM_VMM_H