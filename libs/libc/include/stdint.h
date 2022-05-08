#ifndef _LIBC_STDINT_H
#define _LIBC_STDINT_H

#include <bits/types.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#ifndef __stdints_defined
#define __stdints_defined
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
#endif // __stdints_defined

#ifndef __stdleastints_defined
#define __stdleastints_defined
typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;
typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;
#endif // __stdleastints_defined

// TODO: Optimize for 64bit systems.
#ifndef __stdfastints_defined
#define __stdfastints_defined
typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;
typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;
#endif // __stdfastints_defined

#ifndef __stdptrints_defined
#define __stdptrints_defined
#ifdef BITS64
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;
#elif defined(BITS32)
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#else
#error Unknown BITSx?
#endif
#endif // __stdptrints_defined

#ifndef __stdmaxints_defined
#define __stdmaxints_defined
typedef int64_t intmax_t;
#endif // __stdmaxints_defined

#ifndef __stdintmacroses_defined
#define __stdintmacroses_defined
#define INT8_MAX 0x7f
#define INT8_MIN (-INT8_MAX - 1)
#define UINT8_MAX (INT8_MAX * 2 + 1)

#define INT16_MAX 0x7fff
#define INT16_MIN (-INT16_MAX - 1)
#define UINT16_MAX (INT16_MAX * 2U + 1U)

#define INT32_MAX 0x7fffffffL
#define INT32_MIN (-INT32_MAX - 1L)
#define UINT32_MAX (INT32_MAX * 2UL + 1UL)

#define INT64_MAX 0x7fffffffffffffffLL
#define INT64_MIN (-INT64_MAX - 1LL)
#define UINT64_MAX (INT64_MAX * 2ULL + 1ULL)

#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST8_MIN INT8_MIN
#define UINT_LEAST8_MAX UINT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST16_MIN INT16_MIN
#define UINT_LEAST16_MAX UINT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST32_MIN INT32_MIN
#define UINT_LEAST32_MAX UINT32_MAX
#define INT_LEAST64_MAX INT64_MAX
#define INT_LEAST64_MIN INT64_MIN
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MAX INT8_MAX
#define INT_FAST8_MIN INT8_MIN
#define UINT_FAST8_MAX UINT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST16_MIN INT16_MIN
#define UINT_FAST16_MAX UINT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST32_MIN INT32_MIN
#define UINT_FAST32_MAX UINT32_MAX
#define INT_FAST64_MAX INT64_MAX
#define INT_FAST64_MIN INT64_MIN
#define UINT_FAST64_MAX UINT64_MAX
#endif // __stdintmacroses_defined

__END_DECLS

#endif //_LIBC_STDINT_H