/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <libkern/libkern.h>

static devtree_header_t* devtree_header = NULL;
static devtree_entry_t* devtree_body = NULL;
static char* devtree_name_section = NULL;

int devtree_init(void* devtree)
{
    if (!devtree) {
        return 1;
    }

    devtree_header_t* dth = (devtree_header_t*)devtree;
    if (memcmp(dth->signature, DEVTREE_HEADER_SIGNATURE, DEVTREE_HEADER_SIGNATURE_LEN)) {
        return 1;
    }

    devtree_header = dth;
    devtree_body = (devtree_entry_t*)&dth[1];
    devtree_name_section = ((char*)dth + dth->name_list_offset);
    return 0;
}

devtree_entry_t* devtree_find_device(const char* name)
{
    if (!devtree_body) {
        return NULL;
    }

    for (int i = 0; i < devtree_header->entries_count; i++) {
        const char* curdev_name = &devtree_name_section[devtree_body[i].rel_name_offset];
        if (strcmp(curdev_name, name) == 0) {
            return &devtree_body[i];
        }
    }
    return NULL;
}