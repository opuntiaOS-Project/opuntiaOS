/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

bool pty_slave_can_read(file_t* file, size_t start)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);

    return tty_can_read(&pts->tty, file, start);
}

bool pty_slave_can_write(file_t* file, size_t start)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);

    return sync_ringbuffer_space_to_write(&pts->ptm->buffer) >= 0;
}

int pty_slave_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);

    return tty_read(&pts->tty, file, buf, start, len);
}

int pty_slave_write(file_t* file, void __user* buf, size_t start, size_t len)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);

    sync_ringbuffer_write_user(&pts->ptm->buffer, buf, len);
    return len;
}

int pty_slave_ioctl(file_t* file, uint32_t cmd, uint32_t arg)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_slave_entry_t* pts = _pts_get(dentry);
    ASSERT(pts);

    return tty_ioctl(&pts->tty, file, cmd, arg);
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
        devfs_inode_t* res = devfs_register(mp, MKDEV(136, id), name, 4, S_IFCHR | 0777, &fops);
        pty_slaves[id].inode_indx = res->index;
        pty_slaves[id].ptm = ptm;

        tty_init(&pty_slaves[id].tty);
        ASSERT(pty_slaves[id].tty.buffer.ringbuffer.zone.start);
        ptm->pts = &pty_slaves[id];
    } else {
        tty_clear(&pty_slaves[id].tty);
    }

    dentry_put(mp);
    return 0;
}