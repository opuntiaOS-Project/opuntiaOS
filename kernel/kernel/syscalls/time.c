/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    clockid_t clk_id = param1;
    timespec_t* u_ts = (timespec_t*)param2;

    switch (clk_id) {
    case CLOCK_MONOTONIC:
        u_ts->tv_sec = timeman_seconds_since_boot();
        u_ts->tv_nsec = timeman_get_ticks_from_last_second() * (1000000000 / timeman_ticks_per_second());
        break;
    case CLOCK_REALTIME:
        u_ts->tv_sec = timeman_now();
        u_ts->tv_nsec = timeman_get_ticks_from_last_second() * (1000000000 / timeman_ticks_per_second());
        break;
    default:
        return_with_val(-EINVAL);
    }
    return_with_val(0);
}
