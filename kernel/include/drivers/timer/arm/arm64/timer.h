/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_TIMER_ARM_ARM64_TIMER_H
#define _KERNEL_DRIVERS_TIMER_ARM_ARM64_TIMER_H

#include <libkern/types.h>
#include <time/time_manager.h>

void arm64_timer_rearm();
void arm64_timer_install();

#endif // _KERNEL_DRIVERS_TIMER_ARM_ARM64_TIMER_H