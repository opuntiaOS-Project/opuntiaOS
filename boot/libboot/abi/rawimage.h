/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_ABI_RAWIMAGE_H
#define _BOOT_LIBBOOT_ABI_RAWIMAGE_H

#include <libboot/types.h>

struct rawimage_header {
    uint64_t kern_off;
    uint64_t kern_size;
    uint64_t devtree_off;
    uint64_t devtree_size;
    uint64_t ramdisk_off;
    uint64_t ramdisk_size;
    uint64_t rawimage_size;
    uint64_t padding;
};
typedef struct rawimage_header rawimage_header_t;

#endif // _BOOT_LIBBOOT_ABI_RAWIMAGE_H