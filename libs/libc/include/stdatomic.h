#ifndef _LIBC_STDATOMIC_H
#define _LIBC_STDATOMIC_H

#include <stdbool.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

typedef enum {
    memory_order_relaxed = __ATOMIC_RELAXED,
    memory_order_consume = __ATOMIC_CONSUME,
    memory_order_acquire = __ATOMIC_ACQUIRE,
    memory_order_release = __ATOMIC_RELEASE,
    memory_order_acq_rel = __ATOMIC_ACQ_REL,
    memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

typedef _Atomic bool atomic_bool;
typedef _Atomic char atomic_char;
typedef _Atomic signed char atomic_schar;
typedef _Atomic unsigned char atomic_uchar;
typedef _Atomic short atomic_short;
typedef _Atomic unsigned short atomic_ushort;
typedef _Atomic int atomic_int;
typedef _Atomic unsigned int atomic_uint;
typedef _Atomic long atomic_long;
typedef _Atomic unsigned long atomic_ulong;
typedef _Atomic long long atomic_llong;
typedef _Atomic unsigned long long atomic_ullong;

#define atomic_add_explicit(x, val, ord) (__atomic_add_fetch(x, val, ord))
#define atomic_store_explicit(x, val, ord) (__atomic_store_n(x, val, ord))
#define atomic_load_explicit(x, ord) (__atomic_load_n(x, ord))

#define atomic_add(x, val) (atomic_add_explicit(x, val, __ATOMIC_SEQ_CST))
#define atomic_store(x, val) (atomic_store_explicit(x, val, __ATOMIC_SEQ_CST))
#define atomic_load(x) (atomic_load_explicit(x, __ATOMIC_SEQ_CST))

__END_DECLS

#endif //_LIBC_STDATOMIC_H