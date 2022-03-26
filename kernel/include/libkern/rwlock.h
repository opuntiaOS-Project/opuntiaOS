/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_RWLOCK_H
#define _KERNEL_LIBKERN_RWLOCK_H

#include <libkern/atomic.h>
#include <libkern/c_attrs.h>
#include <libkern/kassert.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <libkern/types.h>

struct rwlock {
    int readers;
    spinlock_t lock;
#ifdef DEBUG_SPINLOCK

#endif // DEBUG_SPINLOCK
};
typedef struct rwlock rwspinlock_t;

static ALWAYS_INLINE void rwspinlock_init(rwspinlock_t* rwlock)
{
}

static ALWAYS_INLINE void rwlock_r_acquire(rwspinlock_t* rwlock)
{
    spinlock_acquire(&rwlock->lock);
    rwlock->readers++;
    spinlock_release(&rwlock->lock);
}

static ALWAYS_INLINE void rwlock_r_release(rwspinlock_t* rwlock)
{
    spinlock_acquire(&rwlock->lock);
    ASSERT(rwlock->readers >= 0);
    rwlock->readers--;
    spinlock_release(&rwlock->lock);
}

static ALWAYS_INLINE void rwlock_w_acquire(rwspinlock_t* rwlock)
{
    for (;;) {
        spinlock_acquire(&rwlock->lock);
        if (!rwlock->readers) {
            return;
        }
        spinlock_release(&rwlock->lock);
    }
}

static ALWAYS_INLINE void rwlock_w_release(rwspinlock_t* rwlock)
{
    spinlock_release(&rwlock->lock);
}

// #define DEBUG_SPINLOCK
#ifdef DEBUG_SPINLOCK
#define rwlock_r_acquire(x)                                    \
    log("acquire r rwlock %s %s:%d ", #x, __FILE__, __LINE__); \
    rwlock_r_acquire(x);

#define rwlock_r_release(x)                                    \
    log("release r rwlock %s %s:%d ", #x, __FILE__, __LINE__); \
    rwlock_r_release(x);

#define rwlock_w_acquire(x)                                    \
    log("acquire w rwlock %s %s:%d ", #x, __FILE__, __LINE__); \
    rwlock_w_acquire(x);

#define rwlock_w_release(x)                                    \
    log("release w rwlock %s %s:%d ", #x, __FILE__, __LINE__); \
    rwlock_w_release(x);
#endif

#endif // _KERNEL_LIBKERN_RWLOCK_H