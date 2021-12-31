/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_KEYBOARD_H
#define _KERNEL_DRIVERS_X86_KEYBOARD_H

#include <drivers/driver_manager.h>
#include <drivers/generic/keyboard.h>

void keyboard_handler();
bool kbdriver_install();
void kbdriver_run();

uint32_t kbdriver_get_last_key();
void kbdriver_discard_last_key();

#endif
