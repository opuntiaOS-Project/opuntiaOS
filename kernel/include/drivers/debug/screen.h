/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_DEBUG_SCREEN_H
#define _KERNEL_DRIVERS_DEBUG_SCREEN_H

#include <libkern/types.h>

struct boot_args;
int screen_setup(struct boot_args* boot_args);
int screen_put_char(char c);

#endif //_KERNEL_DRIVERS_DEBUG_SCREEN_H