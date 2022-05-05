/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH64_TIMER_H
#define _KERNEL_DRIVERS_AARCH64_TIMER_H

#include <libkern/types.h>

#define TIMER_TICKS_PER_SECOND 125

void aarch64_timer_enable();
void aarch64_timer_disable();
void aarch64_timer_rearm();
void aarch64_timer_init();

#endif // _KERNEL_DRIVERS_AARCH64_TIMER_H