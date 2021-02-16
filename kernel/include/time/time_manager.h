/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__X86__TIME__TIME_MANAGER_H
#define __oneOS__X86__TIME__TIME_MANAGER_H

#include <types.h>

/* 32 bits is enough until 2106y */
typedef unsigned int time_t;

struct timeval {
    time_t tv_sec;
    uint32_t tv_usec;
};
typedef struct timeval timeval_t;

bool timeman_is_leap_year(uint32_t year);
uint32_t timeman_days_in_years_since_epoch(uint32_t year);
uint32_t timeman_days_in_months_since_soy(uint8_t month, uint32_t year);
time_t timeman_to_seconds_since_epoch(uint8_t secs, uint8_t mins, uint8_t hrs, uint8_t day, uint8_t month, uint32_t year);

int timeman_setup();
void timeman_pit_tick();

time_t timeman_now();
time_t timeman_seconds_since_boot();
time_t timeman_get_ticks_from_last_second();

#endif /* __oneOS__X86__TIME__TIME_MANAGER_H */
