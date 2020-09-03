/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__TIME__TIME_MANAGER_H
#define __oneOS__X86__TIME__TIME_MANAGER_H

#include <types.h>

typedef long long int time_t;

bool timeman_is_leap_year(uint32_t year);
uint32_t timeman_days_in_years_since_epoch(uint32_t year);
uint32_t timeman_days_in_months_since_soy(uint8_t month, uint32_t year);
time_t timeman_to_seconds_since_epoch(uint8_t secs, uint8_t mins, uint8_t hrs, uint8_t day, uint8_t month, uint32_t year);

int timeman_setup();
void timeman_pit_tick();

time_t timeman_now();
time_t timeman_seconds_since_boot();

#endif /* __oneOS__X86__TIME__TIME_MANAGER_H */
