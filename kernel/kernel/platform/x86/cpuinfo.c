/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <platform/generic/cpu.h>
#include <platform/x86/cpuinfo.h>

struct cpuid {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};
typedef struct cpuid cpuid_t;

static cpuid_t read_cpuid(int eax, int ecx)
{
    cpuid_t res;
    asm volatile("cpuid"
                 : "=a"(res.eax), "=b"(res.ebx), "=c"(res.ecx), "=d"(res.edx)
                 : "a"(eax), "c"(ecx));
    return res;
}

#define SET_FEAT(cpuid_reg, bit, op) \
    if (TEST_BIT(cpuid_reg, bit)) {  \
        op;                          \
    }

void cpuinfo_init()
{
    THIS_CPU->cpufeat = 0x0;

    cpuid_t cpuid_1_0 = read_cpuid(1, 0);
    SET_FEAT(cpuid_1_0.edx, 0, THIS_CPU->cpufeat |= CPUFEAT_FPU);
    SET_FEAT(cpuid_1_0.edx, 3, THIS_CPU->cpufeat |= CPUFEAT_PSE);
    SET_FEAT(cpuid_1_0.edx, 6, THIS_CPU->cpufeat |= CPUFEAT_PAE);
    SET_FEAT(cpuid_1_0.edx, 19, THIS_CPU->cpufeat |= CPUFEAT_CLFSH);
    SET_FEAT(cpuid_1_0.edx, 25, THIS_CPU->cpufeat |= CPUFEAT_SSE);
    SET_FEAT(cpuid_1_0.edx, 26, THIS_CPU->cpufeat |= CPUFEAT_SSE2);

    SET_FEAT(cpuid_1_0.ecx, 0, THIS_CPU->cpufeat |= CPUFEAT_SSE3);
    SET_FEAT(cpuid_1_0.ecx, 9, THIS_CPU->cpufeat |= CPUFEAT_SSSE3);
    SET_FEAT(cpuid_1_0.ecx, 19, THIS_CPU->cpufeat |= CPUFEAT_SSE4_1);
    SET_FEAT(cpuid_1_0.ecx, 20, THIS_CPU->cpufeat |= CPUFEAT_SSE4_2);
    SET_FEAT(cpuid_1_0.ecx, 26, THIS_CPU->cpufeat |= CPUFEAT_XSAVE);
    SET_FEAT(cpuid_1_0.ecx, 28, THIS_CPU->cpufeat |= CPUFEAT_AVX);

    cpuid_t cpuid_ex1_0 = read_cpuid(0x80000001, 0);
    log("cpu %zx %zx", cpuid_ex1_0.edx, (cpuid_ex1_0.edx & (1 << 26)));
    SET_FEAT(cpuid_ex1_0.edx, 26, THIS_CPU->cpufeat |= CPUFEAT_PDPE1GB);
}
