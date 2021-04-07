/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/kmalloc.h>
#include <mem/vmm/vmm.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/tss.h>

tss_t tss;

void ltr(uint16_t seg)
{
    asm volatile("ltr %0"
                 :
                 : "r"(seg));
}
