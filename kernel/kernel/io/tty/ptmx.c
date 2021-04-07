/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/devfs/devfs.h>
#include <io/tty/ptmx.h>
#include <io/tty/pty_master.h>
#include <libkern/libkern.h>
#include <libkern/log.h>

// #define PTY_DEBUG

bool ptmx_can_read(dentry_t* dentry, uint32_t start)
{
    return true;
}

bool ptmx_can_write(dentry_t* dentry, uint32_t start)
{
    return true;
}

int ptmx_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    return 0;
}

int ptmx_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    return 0;
}

int ptmx_open(dentry_t* dentry, struct file_descriptor* fd, uint32_t flags)
{
#ifdef PTY_DEBUG
    log("Opening ptmx");
#endif
    pty_master_alloc(fd);
    return 0;
}

int ptmx_install()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return -1;
    }

    file_ops_t fops = {0};
    fops.open = ptmx_open;
    fops.can_read = ptmx_can_read;
    fops.can_write = ptmx_can_write;
    fops.read = ptmx_read;
    fops.write = ptmx_write;
    devfs_inode_t* res = devfs_register(mp, MKDEV(5, 2), "ptmx", 4, 0, &fops);
    dentry_put(mp);
    return 0;
}
