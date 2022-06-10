/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/kmalloc.h>
#include <mem/vmm.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/tss.h>

gdt_desc_t gdt[GDT_MAX_ENTRIES];

void lgdt(void* p, uint16_t size)
{
    uint16_t pd[3];
    pd[0] = size - 1;
    pd[1] = (uint32_t)p;
    pd[2] = (uint32_t)p >> 16;
    asm volatile("lgdt (%0)"
                 :
                 : "r"(pd));
}

void gdt_setup()
{
    gdt[GDT_SEG_KCODE] = GDT_SEG_PG(GDT_SEGF_X | GDT_SEGF_R, 0, 0xffffffff, 0);
    gdt[GDT_SEG_KDATA] = GDT_SEG_PG(GDT_SEGF_W, 0, 0xffffffff, 0);
    gdt[GDT_SEG_UCODE] = GDT_SEG_PG(GDT_SEGF_X | GDT_SEGF_R, 0, 0xffffffff, DPL_USER);
    gdt[GDT_SEG_UDATA] = GDT_SEG_PG(GDT_SEGF_W, 0, 0xffffffff, DPL_USER);
    lgdt(gdt, sizeof(gdt));
}