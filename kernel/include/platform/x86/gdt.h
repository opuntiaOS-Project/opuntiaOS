/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_GDT_H
#define _KERNEL_PLATFORM_X86_GDT_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

#ifdef __x86_64__
#define GDT_MAX_ENTRIES 7 // TSS takes 2 entries.

// Marking that code segment contains native 64-bit code.
#define GDT_LONGMODE_FLAG 1
#define GDT_DB_FLAG 0
#else
#define GDT_MAX_ENTRIES 6

#define GDT_LONGMODE_FLAG 0
#define GDT_DB_FLAG 1
#endif

#define GDT_SEG_NULL 0 // kernel code
#define GDT_SEG_KCODE 1 // kernel code
#define GDT_SEG_KDATA 2 // kernel data+stack
#define GDT_SEG_UCODE 3 // user code
#define GDT_SEG_UDATA 4 // user data+stack
#define GDT_SEG_TSS 5 // task state NOT USED CURRENTLY

#define GDT_SEGF_X 0x8 // exec
#define GDT_SEGF_A 0x1 // accessed
#define GDT_SEGF_R 0x2 // readable (if exec)
#define GDT_SEGF_C 0x4 // conforming seg (if exec)
#define GDT_SEGF_W 0x2 // writeable (if non-exec)
#define GDT_SEGF_D 0x4 // grows down (if non-exec)

#define FL_IF 0x00000200

#define DPL_KERN 0x0
#define DPL_USER 0x3

struct PACKED gdt_desc {
    union {
        struct {
            uint32_t lim_15_0 : 16;
            uint32_t base_15_0 : 16;
            uint32_t base_23_16 : 8;
            uint32_t type : 4;
            uint32_t dt : 1;
            uint32_t dpl : 2;
            uint32_t p : 1;
            uint32_t lim_19_16 : 4;
            uint32_t avl : 1;
            uint32_t l : 1;
            uint32_t db : 1;
            uint32_t g : 1;
            uint32_t base_31_24 : 8;
        };
        uint32_t raw;
    };
};
typedef struct gdt_desc gdt_desc_t;

extern gdt_desc_t gdt[GDT_MAX_ENTRIES];

#define GDT_SEG_CODE_DESC(vtype, vbase, vlimit, vdpl)   \
    (gdt_desc_t)                                        \
    {                                                   \
        .lim_15_0 = ((vlimit) >> 12) & 0xffff,          \
        .base_15_0 = (uint32_t)(vbase)&0xffff,          \
        .base_23_16 = ((uint32_t)(vbase) >> 16) & 0xff, \
        .type = vtype,                                  \
        .dt = 1,                                        \
        .dpl = vdpl,                                    \
        .p = 1,                                         \
        .lim_19_16 = ((uint32_t)(vlimit) >> 28),        \
        .avl = 0,                                       \
        .l = GDT_LONGMODE_FLAG,                         \
        .db = GDT_DB_FLAG,                              \
        .g = 1,                                         \
        .base_31_24 = (uint32_t)(vbase) >> 24           \
    }

#define GDT_SEG_DATA_DESC(vtype, vbase, vlimit, vdpl)   \
    (gdt_desc_t)                                        \
    {                                                   \
        .lim_15_0 = ((vlimit) >> 12) & 0xffff,          \
        .base_15_0 = (uint32_t)(vbase)&0xffff,          \
        .base_23_16 = ((uint32_t)(vbase) >> 16) & 0xff, \
        .type = vtype,                                  \
        .dt = 1,                                        \
        .dpl = vdpl,                                    \
        .p = 1,                                         \
        .lim_19_16 = ((uint32_t)(vlimit) >> 28),        \
        .avl = 0,                                       \
        .l = 0,                                         \
        .db = 1,                                        \
        .g = 1,                                         \
        .base_31_24 = (uint32_t)(vbase) >> 24           \
    }

#define GDT_SEG_TSS_DESC(vtype, vbase, vlimit, vdpl)    \
    (gdt_desc_t)                                        \
    {                                                   \
        .lim_15_0 = ((vlimit)) & 0xffff,                \
        .base_15_0 = (uint32_t)(vbase)&0xffff,          \
        .base_23_16 = ((uint32_t)(vbase) >> 16) & 0xff, \
        .type = vtype,                                  \
        .dt = 0,                                        \
        .dpl = vdpl,                                    \
        .p = 1,                                         \
        .lim_19_16 = ((uint32_t)(vlimit) >> 16),        \
        .avl = 0,                                       \
        .l = 0,                                         \
        .db = 0,                                        \
        .g = 0,                                         \
        .base_31_24 = (uint32_t)(vbase) >> 24           \
    }

#define GDT_SEG_SET_RAW(rawvalue) \
    (gdt_desc_t)                  \
    {                             \
        .raw = rawvalue           \
    }

void gdt_setup();

#endif // _KERNEL_PLATFORM_X86_GDT_H