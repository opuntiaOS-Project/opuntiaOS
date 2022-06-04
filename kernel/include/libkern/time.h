/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_TIME_H
#define _KERNEL_LIBKERN_TIME_H

#include <libkern/bits/time.h>
#include <libkern/kassert.h>
#include <libkern/types.h>

static inline void timespec_add_nsec(timespec_t* time, int nsec)
{
    ASSERT(-1000000000 < nsec && nsec < 1000000000);

    time->tv_nsec += nsec;
    if (time->tv_nsec >= 1000000000) {
        time->tv_nsec -= 1000000000;
        time->tv_sec++;
    }
    if (time->tv_nsec < 0) {
        time->tv_nsec += 1000000000;
        time->tv_sec--;
    }
}

static inline void timespec_add_usec(timespec_t* time, int usec)
{
    int sec = usec / 1000000;
    int nsec = (usec % 1000000) * 1000;
    timespec_add_nsec(time, nsec);
    time->tv_sec += sec;
}

static inline void timespec_add_sec(timespec_t* time, int sec)
{
    time->tv_sec += sec;
}

static inline void timespec_add_timespec(timespec_t* lhs, const timespec_t* rhs)
{
    timespec_add_nsec(lhs, rhs->tv_nsec);
    lhs->tv_sec += rhs->tv_sec;
}

static inline void timespec_add_timeval(timespec_t* lhs, const timeval_t* rhs)
{
    timespec_add_usec(lhs, rhs->tv_usec);
    lhs->tv_sec += rhs->tv_sec;
}

static inline int timespec_cmp(const timespec_t* a, const timespec_t* b)
{
    if (a->tv_sec == b->tv_sec) {
        if (a->tv_nsec == b->tv_nsec) {
            return 0;
        }
        if (a->tv_nsec < b->tv_nsec) {
            return -1;
        }
        return 1;
    }
    if (a->tv_sec < b->tv_sec) {
        return -2;
    }
    return 2;
}

#endif // _KERNEL_LIBKERN_TIME_H