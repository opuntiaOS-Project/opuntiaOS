#pragma once

#include <sys/types.h>

[[gnu::always_inline]] inline void fast_copy(uint32_t* dest, const uint32_t* src, size_t count)
{
#ifdef __i386__
    asm volatile(
        "rep movsl\n"
        : "=S"(src), "=D"(dest), "=c"(count)
        : "S"(src), "D"(dest), "c"(count)
        : "memory");
#elif __arm__
    for (size_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
#endif
}

[[gnu::always_inline]] inline void fast_set(uint32_t* dest, uint32_t val, size_t count)
{
#ifdef __i386__
    asm volatile(
        "rep stosl\n"
        : "=D"(dest), "=c"(count)
        : "D"(dest), "c"(count), "a"(val)
        : "memory");
#elif __arm__
    for (size_t i = 0; i < count; i++) {
        dest[i] = val;
    }
#endif
}