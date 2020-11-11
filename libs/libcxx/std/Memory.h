#pragma once

#include <sys/types.h>

[[gnu::always_inline]] inline void fast_copy(uint32_t* dest, const uint32_t* src, size_t count)
{
    asm volatile(
        "rep movsl\n"
        : "=S"(src), "=D"(dest), "=c"(count)
        : "S"(src), "D"(dest), "c"(count)
        : "memory");
}