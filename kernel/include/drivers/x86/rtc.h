/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_RTC_H
#define _KERNEL_DRIVERS_X86_RTC_H

#include <libkern/types.h>

void rtc_load_time(uint8_t* secs, uint8_t* mins, uint8_t* hrs, uint8_t* day, uint8_t* month, uint32_t* year);

#endif /* _KERNEL_DRIVERS_X86_RTC_H */