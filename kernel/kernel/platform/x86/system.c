/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <platform/x86/system.h>
// #include <tasking/tasking.h>

bool system_can_preempt_kernel()
{
    // TODO(x64): #ifdef PREEMPT_KERNEL
    //     return THIS_CPU->int_depth_counter == 0;
    // #else
    return 0;
    // #endif
}

void system_disable_interrupts()
{
    // TODO(x64):THIS_CPU->int_depth_counter++;
    system_disable_interrupts_no_counter();
}

void system_enable_interrupts()
{
    // TODO(x64):THIS_CPU->int_depth_counter--;
    // ASSERT(THIS_CPU->int_depth_counter >= 0);
    // if (THIS_CPU->int_depth_counter == 0) {
        system_enable_interrupts_no_counter();
    // }
}

void system_enable_interrupts_only_counter()
{
    // TODO(x64):THIS_CPU->int_depth_counter--;
    // ASSERT(THIS_CPU->int_depth_counter >= 0);
}

void system_cache_clean_and_invalidate(void* addr, size_t size)
{
    const size_t cache_line_size = 64;
    size_t start = ROUND_FLOOR((size_t)addr, cache_line_size);
    size_t end = ROUND_CEIL((size_t)addr + size, cache_line_size);

    for (size_t curaddr = start; curaddr < end; curaddr += cache_line_size) {
        // TODO(x86) This is commented out until cpuid flags are avail.
        // asm volatile("clflush (%0)"
        //              :
        //              : "r"(curaddr)
        //              : "memory");
    }
}

void system_cache_invalidate(void* addr, size_t size)
{
    return system_cache_clean_and_invalidate(addr, size);
}

void system_cache_clean(void* addr, size_t size)
{
    return system_cache_clean_and_invalidate(addr, size);
}