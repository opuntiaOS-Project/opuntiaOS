/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/devfs/devfs.h>
#include <io/tty/pty_master.h>
#include <io/tty/pty_slave.h>
#include <libkern/libkern.h>
#include <libkern/log.h>

pty_slave_entry_t pty_slaves[PTYS_COUNT];

static pty_slave_entry_t* _pts_get(dentry_t* dentry)
{
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (dentry->inode_indx == pty_slaves[i].inode_indx) {
            return &pty_slaves[i];
        }
    }
    return NULL;
}

bool pty_slave_can_read(dentry_t* dentry, uint32_t start)
{
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);
    return sync_ringbuffer_space_to_read(&pts->buffer) >= 1;
}

bool pty_slave_can_write(dentry_t* dentry, uint32_t start)
{
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);
    return sync_ringbuffer_space_to_write(&pts->ptm->buffer) >= 0;
}

int pty_slave_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);
    uint32_t leno = sync_ringbuffer_space_to_read(&pts->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = sync_ringbuffer_read(&pts->buffer, buf, leno);
    return leno;
}

int pty_slave_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);
    sync_ringbuffer_write(&pts->ptm->buffer, buf, len);
    return len;
}

int pty_slave_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    return 0;
}

int pty_slave_create(int id, pty_master_entry_t* ptm)
{
    ASSERT(0 <= id && id < 10 && id <= PTYS_COUNT);

    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return 0;
    }

    // FIXME: Check if the we already have pty file created
    if (!pty_slaves[id].inode_indx) {
        char name[8];
        snprintf(name, 8, "pts%d", id);
        file_ops_t fops = { 0 };
        fops.can_read = pty_slave_can_read;
        fops.can_write = pty_slave_can_write;
        fops.read = pty_slave_read;
        fops.write = pty_slave_write;
        fops.ioctl = pty_slave_ioctl;
        devfs_inode_t* res = devfs_register(mp, MKDEV(136, id), name, 4, 0, &fops);
        pty_slaves[id].inode_indx = res->index;
        pty_slaves[id].ptm = ptm;
        pty_slaves[id].buffer = sync_ringbuffer_create_std();
        ASSERT(pty_slaves[id].buffer.ringbuffer.zone.start);
        ptm->pts = &pty_slaves[id];
    } else {
        pty_slaves[id].buffer.ringbuffer.start = pty_slaves[id].buffer.ringbuffer.end = 0;
    }

    dentry_put(mp);
    return 0;
}