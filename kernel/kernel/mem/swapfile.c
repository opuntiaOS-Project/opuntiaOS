/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <mem/swapfile.h>
#include <platform/generic/cpu.h>

static file_t* _swapfile = NULL;
static int _nextid = 0;

int swapfile_init()
{
    dentry_t* var_dir = NULL;
    if (vfs_resolve_path("/var", &var_dir) < 0) {
        // Instead of panicing we can create the dir here.
        kpanic("No /var dir is found");
    }

    vfs_create(var_dir, "swapfile", 8, 0600, 0, 0);

    dentry_t* swapfile_dentry;
    if (vfs_resolve_path("/var/swapfile", &swapfile_dentry) < 0) {
        return -1;
    }

    _swapfile = file_init_dentry_move(swapfile_dentry);
    return 0;
}

int swapfile_new_ref(int id)
{
    return 0;
}

int swapfile_rem_ref(int id)
{
    return 0;
}

int swapfile_load(uintptr_t vaddr, int id)
{
    if (!_swapfile) {
        return -ENODEV;
    }

    if (id > _nextid) {
        return -ENOENT;
    }

    data_access_type_t prev_access_type = THIS_CPU->data_access_type;
    THIS_CPU->data_access_type = DATA_ACCESS_KERNEL;

    size_t offset = (id - 1) * VMM_PAGE_SIZE;
    _swapfile->ops->read(_swapfile, (void*)PAGE_START(vaddr), offset, VMM_PAGE_SIZE);

    THIS_CPU->data_access_type = prev_access_type;

    swapfile_rem_ref(id);
    return 0;
}

int swapfile_store(uintptr_t vaddr)
{
    if (!_swapfile) {
        return -ENODEV;
    }

    data_access_type_t prev_access_type = THIS_CPU->data_access_type;
    THIS_CPU->data_access_type = DATA_ACCESS_KERNEL;

    _swapfile->ops->write(_swapfile, (void*)PAGE_START(vaddr), _nextid * VMM_PAGE_SIZE, VMM_PAGE_SIZE);
    _nextid++;

    THIS_CPU->data_access_type = prev_access_type;

    swapfile_new_ref(_nextid);
    return _nextid;
}