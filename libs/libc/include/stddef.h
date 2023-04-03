#ifndef _LIBC_STDDEF_H
#define _LIBC_STDDEF_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifdef __i386__
#if defined(__clang__)
typedef unsigned int size_t;
typedef int ssize_t;
typedef int ptrdiff_t;
#elif defined(__GNUC__) || defined(__GNUG__)
typedef unsigned long size_t;
typedef long ssize_t;
typedef long ptrdiff_t;
#endif
#elif __x86_64__
typedef unsigned long size_t;
typedef long long ssize_t;
typedef long long ptrdiff_t;
#elif __arm__
typedef unsigned int size_t;
typedef int ssize_t;
typedef int ptrdiff_t;
#elif __aarch64__
typedef unsigned long size_t;
typedef long long ssize_t;
typedef long long ptrdiff_t;
#elif defined(__riscv) && (__riscv_xlen == 64)
typedef unsigned long size_t;
typedef long long ssize_t;
typedef long long ptrdiff_t;
#endif

#define NULL ((void*)0)

#define offsetof(type, member) __builtin_offsetof(type, member)

__END_DECLS

#endif // _LIBC_STDDEF_H