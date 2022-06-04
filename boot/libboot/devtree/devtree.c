/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "devtree.h"
#include <libboot/log/log.h>
#include <libboot/mem/mem.h>

static size_t _devtree_size = 0;
static devtree_header_t* _devtree_header = NULL;
static devtree_entry_t* _devtree_body = NULL;
static char* _devtree_name_section = NULL;

void* devtree_start()
{
    return (void*)_devtree_header;
}

size_t devtree_size()
{
    return _devtree_size;
}

int devtree_init(void* devtree, size_t size)
{
    if (!devtree) {
        return 0;
    }

    devtree_header_t* dth = (devtree_header_t*)devtree;
    _devtree_header = dth;
    _devtree_body = (devtree_entry_t*)&dth[1];
    _devtree_name_section = ((char*)dth + dth->name_list_offset);
    _devtree_size = size;
    return 0;
}

const char* devtree_name_of_entry(devtree_entry_t* en)
{
    return &_devtree_name_section[en->rel_name_offset];
}

devtree_entry_t* devtree_find_device(const char* name)
{
    if (!_devtree_body) {
        return NULL;
    }

    for (int i = 0; i < _devtree_header->entries_count; i++) {
        const char* curdev_name = devtree_name_of_entry(&_devtree_body[i]);
        if (strcmp(curdev_name, name) == 0) {
            return &_devtree_body[i];
        }
    }
    return NULL;
}
