/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__DRIVERS__PIT_H
#define __oneOS__DRIVERS__PIT_H

#include <types.h>
#include <platform/x86/idt.h>

#define PIT_BASE_FREQ 1193180
#define TIMER_TICKS_PER_SECOND 125

void pit_setup();
void pit_handler();

#endif /* __oneOS__DRIVERS__PIT_H */

