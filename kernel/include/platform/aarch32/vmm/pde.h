/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_VMM_PDE_H
#define _KERNEL_PLATFORM_AARCH32_VMM_PDE_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

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

#define TABLE_DESC_FRAME_OFFSET 10

#endif //_KERNEL_PLATFORM_AARCH32_VMM_PDE_H
