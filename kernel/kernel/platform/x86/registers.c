/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* FIXME: Only for one cpu now */

#include <platform/x86/registers.h>

uint32_t read_cr2()
{
    uint32_t val;
    asm volatile("movl %%cr2,%0"
                 : "=r"(val));
    return val;
}

uint32_t read_cr3()
{
    uint32_t val;
    asm volatile("movl %%cr3,%0"
                 : "=r"(val));
    return val;
}

uint32_t read_esp()
{
    uint32_t val;
    asm volatile("movl %%esp,%0"
                 : "=r"(val));
    return val;
}
