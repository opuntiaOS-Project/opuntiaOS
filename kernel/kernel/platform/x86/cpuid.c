/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/cpuid.h>

cpuid_t get_cpuid(int eax, int ecx)
{
    cpuid_t res;
    asm volatile("cpuid"
                 : "=a"(res.eax), "=b"(res.ebx), "=c"(res.ecx), "=d"(res.edx)
                 : "a"(eax), "c"(ecx));
    return res;
}