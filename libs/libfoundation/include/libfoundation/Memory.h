/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    while (count--) {
        *dest++ = val;
    }
#endif
}
} // namespace LFoundation