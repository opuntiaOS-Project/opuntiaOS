/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <mem/swapfile.h>

static dentry_t* _swapfile = NULL;
static int _nextid = 0;

int swapfile_init()
{
    dentry_t* var_dir = NULL;
    if (vfs_resolve_path("/var", &var_dir) < 0) {
        // Instead of panicing we can create the dir here.
        kpanic("No /var dir is found");
    }

    vfs_create(var_dir, "swapfile", 8, 600, 0, 0);
    if (vfs_resolve_path("/var/swapfile", &_swapfile) < 0) {
        _swapfile = NULL;
    }

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

    size_t offset = (id - 1) * VMM_PAGE_SIZE;
    _swapfile->ops->file.read(_swapfile, (void*)PAGE_START(vaddr), offset, VMM_PAGE_SIZE);
    return 0;
}

int swapfile_store(uintptr_t vaddr)
{
    if (!_swapfile) {
        return -ENODEV;
    }

    _swapfile->ops->file.write(_swapfile, (void*)PAGE_START(vaddr), _nextid * VMM_PAGE_SIZE, VMM_PAGE_SIZE);
    _nextid++;
    return _nextid;
}