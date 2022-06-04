/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_TYPES_H
#define _BOOT_LIBBOOT_TYPES_H

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifdef __i386__
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#if defined(__clang__)
typedef unsigned int size_t;
typedef int ssize_t;
#elif defined(__GNUC__) || defined(__GNUG__)
typedef unsigned long size_t;
typedef long ssize_t;
#endif
#define BITS32
#elif __arm__
typedef unsigned int size_t;
typedef int ssize_t;
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#define BITS32
#elif __aarch64__
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;
#define BITS64
#endif

typedef _Bool bool;

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#define true (1)
#define false (0)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define NULL ((void*)0)

#define UNUSED(x) (void)(x)
#define ALIGNED(x) __attribute__((aligned(x)))
#define PACKED __attribute__((packed))

#ifndef max
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif /* max */

#ifndef min
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif /* min */

#define ROUND_CEIL(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUND_FLOOR(a, b) ((a) & ~((b)-1))

#endif // _BOOT_LIBBOOT_TYPES_H