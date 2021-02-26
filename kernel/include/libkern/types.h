#ifndef _KERNEL_LIBKERN_TYPES_H
#define _KERNEL_LIBKERN_TYPES_H

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef long           int64_t;
typedef int            int32_t;
typedef short          int16_t;
typedef char           int8_t;
typedef char           bool;

#ifdef __i386__
typedef unsigned long size_t;
#elif __arm__
typedef unsigned int size_t;
#endif

#define true  1
#define false 0

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define NULL ((void*)0)

#endif // _KERNEL_LIBKERN_TYPES_H