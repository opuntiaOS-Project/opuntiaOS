/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/gdt.h>

gdt_desc_t gdt[GDT_MAX_ENTRIES];

void lgdt(void* ptr, uint16_t size)
{
    uintptr_t p = (uintptr_t)ptr;
#ifdef __i386__
    // Need to use volatile as GCC optimizes out the following writes.
    volatile uint16_t pd[3];
    pd[0] = size - 1;
    pd[1] = p & 0xffff;
    pd[2] = (p >> 16) & 0xffff;
    asm volatile("lgdt (%0)"
                 :
                 : "r"(pd));
#elif __x86_64__
    // Need to use volatile as GCC optimizes out the following writes.
    volatile uint16_t pd[5];
    pd[0] = size - 1;
    pd[1] = p & 0xffff;
    pd[2] = (p >> 16) & 0xffff;
    pd[3] = (p >> 32) & 0xffff;
    pd[4] = (p >> 48) & 0xffff;
    asm volatile("lgdt (%0)"
                 :
                 : "r"(pd));
#endif
}

void gdt_setup()
{
    gdt[GDT_SEG_KCODE] = GDT_SEG_CODE_DESC(GDT_SEGF_X | GDT_SEGF_R, 0, 0xffffffff, 0);
    gdt[GDT_SEG_KDATA] = GDT_SEG_DATA_DESC(GDT_SEGF_W, 0, 0xffffffff, 0);
    gdt[GDT_SEG_UCODE] = GDT_SEG_CODE_DESC(GDT_SEGF_X | GDT_SEGF_R, 0, 0xffffffff, DPL_USER);
    gdt[GDT_SEG_UDATA] = GDT_SEG_DATA_DESC(GDT_SEGF_W, 0, 0xffffffff, DPL_USER);
    lgdt(gdt, sizeof(gdt));
}