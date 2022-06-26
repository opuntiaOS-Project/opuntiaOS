/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_DRIVERS_SCREEN_H
#define _BOOT_DRIVERS_SCREEN_H

#include <libboot/abi/memory.h>
#include <libboot/types.h>

int fb_init();
int fb_reinit_after_map(uintptr_t vaddr);
int fb_put_char(uint8_t c);

#endif // _BOOT_DRIVERS_SCREEN_H