/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstddef>

namespace LFoundation {

[[gnu::always_inline]] inline void fast_copy(uint32_t* dest, const uint32_t* src, std::size_t count)
{
#ifdef __i386__
    asm volatile(
        "rep movsl\n"
        : "=S"(src), "=D"(dest), "=c"(count)
        : "S"(src), "D"(dest), "c"(count)
        : "memory");
#elif __arm__
    while (count--) {
        asm("pld [%0, #128]" ::"r"(src));
        *dest++ = *src++;
    }
#elif __aarch64__
    while (count--) {
        *dest++ = *src++;
    }
#endif
}

[[gnu::always_inline]] inline void fast_set(uint32_t* dest, uint32_t val, std::size_t count)
{
#ifdef __i386__
    asm volatile(
        "rep stosl\n"
        : "=D"(dest), "=c"(count)
        : "D"(dest), "c"(count), "a"(val)
        : "memory");
#elif __arm__
    asm volatile(
        "cmp     %[count], #0\n"
        "beq     fast_set_exit%=\n"
        "tst     %[ptr], #15\n"
        "beq     fast_set_16bytes_aligned_entry%=\n"
        "fast_set_4bytes_aligned_loop%=:\n"
        "subs	 %[count], %[count], #1\n"
        "str     %[value], [%[ptr]], #4\n"
        "beq     fast_set_exit%=\n"
        "tst     %[ptr], #15\n"
        "bne     fast_set_4bytes_aligned_loop%=\n"
        "fast_set_16bytes_aligned_entry%=:\n"
        "cmp     %[count], #4\n"
        "blt     fast_set_16bytes_aligned_exit%=\n"
        "fast_set_16bytes_aligned_preloop%=:\n"
        "mov     r4, %[value]\n"
        "mov     r5, %[value]\n"
        "mov     r6, %[value]\n"
        "mov     r7, %[value]\n"
        "fast_set_16bytes_aligned_loop%=:\n"
        "subs	 %[count], %[count], #4\n"
        "stmia	 %[ptr]!, {r4,r5,r6,r7}\n"
        "cmp     %[count], #4\n"
        "bge     fast_set_16bytes_aligned_loop%=\n"
        "fast_set_16bytes_aligned_exit%=:\n"
        "cmp     %[count], #0\n"
        "beq     fast_set_exit%=\n"
        "fast_set_4bytes_aligned_loop_2_%=:\n"
        "subs	 %[count], %[count], #1\n"
        "str     %[value], [%[ptr]], #4\n"
        "bne     fast_set_4bytes_aligned_loop_2_%=\n"
        "fast_set_exit%=:"
        : [value] "=r"(val),
        [ptr] "=r"(dest),
        [count] "=r"(count)
        : "[value]"(val),
        "[ptr]"(dest),
        "[count]"(count)
        : "r4", "r5", "r6", "r7", "memory", "cc");
#elif __aarch64__
    while (count--) {
        *dest++ = val;
    }
#endif
}
} // namespace LFoundation