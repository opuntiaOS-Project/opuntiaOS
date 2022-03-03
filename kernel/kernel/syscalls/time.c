/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/generic/syscalls/params.h>
#include <platform/generic/tasking/trapframe.h>
#include <syscalls/handlers.h>
#include <time/time_manager.h>

void sys_clock_gettime(trapframe_t* tf)
{
    clockid_t clk_id = SYSCALL_VAR1(tf);
    timespec_t __user* u_ts = (timespec_t __user*)SYSCALL_VAR2(tf);

    timespec_t kts = { 0 };

    switch (clk_id) {
    case CLOCK_MONOTONIC:
        kts.tv_sec = timeman_seconds_since_boot();
        kts.tv_nsec = timeman_get_ticks_from_last_second() * (1000000000 / timeman_ticks_per_second());
        break;
    case CLOCK_REALTIME:
        kts.tv_sec = timeman_now();
        kts.tv_nsec = timeman_get_ticks_from_last_second() * (1000000000 / timeman_ticks_per_second());
        break;
    default:
        return_with_val(-EINVAL);
    }

    umem_put_user(kts, u_ts);
    return_with_val(0);
}

void sys_gettimeofday(trapframe_t* tf)
{
    timeval_t ktv;
    timezone_t ktz;

    timeval_t __user* tv = (timeval_t __user*)SYSCALL_VAR1(tf);
    timezone_t __user* tz = (timezone_t __user*)SYSCALL_VAR2(tf);

    if (!tv || !tz) {
        return_with_val(-EINVAL);
    }

    ktv.tv_sec = timeman_now();
    ktv.tv_usec = timeman_get_ticks_from_last_second() * (1000000 / timeman_ticks_per_second());

    ktz.tz_dsttime = DST_NONE;
    ktz.tz_minuteswest = 0;

    umem_put_user(ktv, tv);
    umem_put_user(ktz, tz);
    return_with_val(0);
}