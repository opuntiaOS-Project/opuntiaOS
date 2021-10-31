/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
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
    lock_t lock;
#ifdef DEBUG_LOCK

#endif // DEBUG_LOCK
};
typedef struct rwlock rwlock_t;

static ALWAYS_INLINE void rwlock_init(rwlock_t* rwlock)
{
}

static ALWAYS_INLINE void rwlock_r_acquire(rwlock_t* rwlock)
{
    lock_acquire(&rwlock->lock);
    rwlock->readers++;
    lock_release(&rwlock->lock);
}

static ALWAYS_INLINE void rwlock_r_release(rwlock_t* rwlock)
{
    lock_acquire(&rwlock->lock);
    ASSERT(rwlock->readers >= 0);
    rwlock->readers--;
    lock_release(&rwlock->lock);
}

static ALWAYS_INLINE void rwlock_w_acquire(rwlock_t* rwlock)
{
    for (;;) {
        lock_acquire(&rwlock->lock);
        if (!rwlock->readers) {
            return;
        }
        lock_release(&rwlock->lock);
    }
}

static ALWAYS_INLINE void rwlock_w_release(rwlock_t* rwlock)
{
    lock_release(&rwlock->lock);
}

// #define DEBUG_LOCK
#ifdef DEBUG_LOCK
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