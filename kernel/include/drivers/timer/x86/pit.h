/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_TIMER_X86_PIT_H
#define _KERNEL_DRIVERS_TIMER_X86_PIT_H

#include <libkern/types.h>
#include <time/time_manager.h>

#define PIT_BASE_FREQ 1193180

void pit_setup();
void pit_handler();

#endif /* _KERNEL_DRIVERS_TIMER_X86_PIT_H */
