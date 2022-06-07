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
#include <libkern/time.h>
#include <platform/generic/syscalls/params.h>
#include <platform/generic/tasking/trapframe.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>
#include <time/time_manager.h>

void sys_clock_gettime(trapframe_t* tf)
{
    clockid_t clk_id = SYSCALL_VAR1(tf);
    timespec_t __user* u_ts = (timespec_t __user*)SYSCALL_VAR2(tf);

    timespec_t kts = { 0 };

    switch (clk_id) {
    case CLOCK_MONOTONIC:
        kts = timeman_timespec_since_boot();
        break;
    case CLOCK_REALTIME:
        kts = timeman_timespec_since_epoch();
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

    ktv.tv_sec = timeman_seconds_since_epoch();
    ktv.tv_usec = timeman_get_ticks_from_last_second() * (1000000 / timeman_ticks_per_second());

    ktz.tz_dsttime = DST_NONE;
    ktz.tz_minuteswest = 0;

    umem_put_user(ktv, tv);
    umem_put_user(ktz, tz);
    return_with_val(0);
}

void sys_sleep(trapframe_t* tf)
{
    thread_t* p = RUNNING_THREAD;

    timespec_t __user* ureq = (timespec_t __user*)SYSCALL_VAR1(tf);
    timespec_t __user* urem = (timespec_t __user*)SYSCALL_VAR2(tf);

    timespec_t kreq = { 0 };
    timespec_t krem = { 0 };

    if (!ureq) {
        return_with_val(-EINVAL);
    }
    umem_get_user(&kreq, ureq);

    timespec_t ts = timeman_timespec_since_epoch();

    // Inlined timespec_add_timespec(&ts, &kreq); since GCC inlines this
    // functions badly producing incorrect asm as of now.
    timespec_add_nsec(&ts, kreq.tv_nsec);
    ts.tv_sec += kreq.tv_sec;

    init_sleep_blocker(p, ts);

    if (urem) {
        umem_put_user(krem, urem);
    }
    return_with_val(0);
}