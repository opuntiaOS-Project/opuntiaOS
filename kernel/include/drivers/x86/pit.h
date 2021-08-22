/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_PIT_H
#define _KERNEL_DRIVERS_X86_PIT_H

#include <libkern/types.h>
#include <platform/x86/idt.h>

#define PIT_BASE_FREQ 1193180
#define TIMER_TICKS_PER_SECOND 125

void pit_setup();
void pit_handler();

#endif /* _KERNEL_DRIVERS_X86_PIT_H */
