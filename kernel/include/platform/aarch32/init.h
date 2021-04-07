/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_INIT_H
#define _KERNEL_PLATFORM_AARCH32_INIT_H

#include <libkern/types.h>

void platform_setup();
void platform_drivers_setup();

#endif /* _KERNEL_PLATFORM_AARCH32_INIT_H */