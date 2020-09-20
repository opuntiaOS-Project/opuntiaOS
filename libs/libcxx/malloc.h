#pragma once

extern "C" {
    #include <libc/malloc.h>
}

typedef unsigned long size_t;

inline void* operator new(size_t size)
{
    return malloc(size);
}

inline void operator delete(void* ptr)
{
    return free(ptr);
}

inline void operator delete(void* ptr, size_t)
{
    return free(ptr);
}

inline void* operator new[](size_t size)
{
    return malloc(size);
}

inline void operator delete[](void* ptr)
{
    return free(ptr);
}

inline void operator delete[](void* ptr, size_t)
{
    return free(ptr);
}

inline void* operator new(size_t, void* ptr)
{
    return ptr;
}

inline void* operator new[](size_t, void* ptr)
{
    return ptr;
}