/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_ABI_KERNEL_H
#define _BOOT_LIBBOOT_ABI_KERNEL_H

#include <libboot/types.h>

// Bootloaders and Kernel are tight together and share ABI in order
// to run kernel, see docs/boot.md for more information.

static size_t shadow_area_size() { return 4 << 20; }

#endif // _BOOT_LIBBOOT_ABI_KERNEL_H