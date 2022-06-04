/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TIME_TIME_MANAGER_H
#define _KERNEL_TIME_TIME_MANAGER_H

#include <drivers/generic/timer.h>
#include <libkern/atomic.h>
#include <libkern/bits/time.h>
#include <libkern/types.h>
#include <platform/generic/cpu.h>

extern time_t ticks_since_boot;
extern time_t ticks_since_second;

bool timeman_is_leap_year(uint32_t year);
uint32_t timeman_days_in_years_since_epoch(uint32_t year);
uint32_t timeman_days_in_months_since_soy(uint8_t month, uint32_t year);
time_t timeman_to_seconds_since_epoch(uint8_t secs, uint8_t mins, uint8_t hrs, uint8_t day, uint8_t month, uint32_t year);

int timeman_setup();
void timeman_timer_tick();

time_t timeman_seconds_since_epoch();
time_t timeman_seconds_since_boot();
time_t timeman_get_ticks_from_last_second();
timespec_t timeman_timespec_since_epoch();
timespec_t timeman_timespec_since_boot();
timeval_t timeman_timeval_since_epoch();
timeval_t timeman_timeval_since_boot();
static inline time_t timeman_ticks_per_second() { return TIMER_TICKS_PER_SECOND; };
static inline time_t timeman_ticks_since_boot() { return THIS_CPU->stat_ticks_since_boot; };

#endif /* _KERNEL_TIME_TIME_MANAGER_H */
