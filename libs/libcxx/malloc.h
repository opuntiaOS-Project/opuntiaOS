#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <libc/malloc.h>
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#ifdef __i386__
typedef unsigned long size_t;
#elif __arm__
typedef unsigned int size_t;
#endif

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

template <class T, class... Args>
inline T& create(Args&&... args)
{
    T* newobject = new T(args...);
    return *newobject;
}

#endif