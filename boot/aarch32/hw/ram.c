/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ram.h"
#include <libboot/devtree/devtree.h>
#include <libboot/log/log.h>

size_t hw_ram_get_size()
{
    devtree_entry_t* dev = devtree_find_device("ram");
    if (!dev) {
        log("Can't find RAM in devtree");
        while (1) { };
    }
    log("ram size %x", dev->region_base);
    return dev->region_size;
}