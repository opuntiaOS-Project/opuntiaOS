/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_LOCK_H
#define _KERNEL_LIBKERN_LOCK_H

#include <libkern/c_attrs.h>
#include <libkern/kassert.h>
#include <libkern/log.h>
#include <libkern/types.h>

// #define DEBUG_LOCK

struct lock {
    int status;
#ifdef DEBUG_LOCK

#endif // DEBUG_LOCK
};
typedef struct lock lock_t;

static ALWAYS_INLINE void lock_init(lock_t* lock)
{
    __atomic_store_n(&lock->status, 0, __ATOMIC_RELAXED);
}

static ALWAYS_INLINE void lock_acquire(lock_t* lock)
{
    while (__atomic_exchange_n(&lock->status, 1, __ATOMIC_ACQUIRE) == 1) {
        // TODO: May be some cpu sleep?
    }
}

static ALWAYS_INLINE void lock_release(lock_t* lock)
{
    ASSERT(lock->status == 1);
    __atomic_store_n(&lock->status, 0, __ATOMIC_RELEASE);
}

#ifdef DEBUG_LOCK
#define lock_acquire(x)                                    \
    log("acquire lock %s %s:%d ", #x, __FILE__, __LINE__); \
    lock_acquire(x);

#define lock_release(x)                                    \
    log("release lock %s %s:%d ", #x, __FILE__, __LINE__); \
    lock_release(x);
#endif

#endif // _KERNEL_LIBKERN_LOCK_H