/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_IO_X86_KEYBOARD_H
#define _KERNEL_DRIVERS_IO_X86_KEYBOARD_H

#include <drivers/io/keyboard.h>
#include <libkern/types.h>

void keyboard_handler();
void kbdriver_install();
int kbdriver_run();

uint32_t kbdriver_get_last_key();
void kbdriver_discard_last_key();

#endif
