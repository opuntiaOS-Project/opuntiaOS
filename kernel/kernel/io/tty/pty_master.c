/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <io/tty/pty_master.h>
#include <io/tty/pty_slave.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>

#define INODE2PTSNO(x) (x - 1)
#define PTSNO2INODE(x) (x + 1)

/**
 * Since pty masters are virtual files and don't present on real hd,
 * we use an array of dentries. This dentries won't be present in the
 * list of dentries (dentry.c), so other threads con't access it.
 */
pty_master_entry_t pty_masters[PTYS_COUNT];

int _pty_master_free_dentry_data(dentry_t* dentry);
bool pty_master_can_read(file_t* file, size_t start);
bool pty_master_can_write(file_t* file, size_t start);
int pty_master_read(file_t* file, void __user* buf, size_t start, size_t len);
int pty_master_write(file_t* file, void __user* buf, size_t start, size_t len);
int pty_master_fstat(file_t* file, stat_t* stat);

static fs_ops_t pty_master_ops = {
    .recognize = NULL,
    .prepare_fs = NULL,
    .eject_device = NULL,
    .dentry = {
        .read_inode = NULL,
        .write_inode = NULL,
        .free_inode = _pty_master_free_dentry_data,
        .get_fsdata = NULL,
    },
    .file = {
        .can_read = pty_master_can_read,
        .can_write = pty_master_can_write,
        .read = pty_master_read,
        .write = pty_master_write,
        .open = NULL,
        .truncate = NULL,
        .create = NULL,
        .unlink = NULL,
        .getdents = NULL,
        .lookup = NULL,
        .mkdir = NULL,
        .rmdir = NULL,
        .fstat = pty_master_fstat,
        .ioctl = NULL,
        .mmap = NULL,
    }
};

static pty_master_entry_t* _ptm_get(dentry_t* dentry)
{
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (dentry == &pty_masters[i].dentry) {
            return &pty_masters[i];
        }
    }
    return NULL;
}

int _pty_master_free_dentry_data(dentry_t* dentry)
{
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    ptm->dentry.inode_indx = 0;
    return 0;
}

bool pty_master_can_read(file_t* file, size_t start)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    return sync_ringbuffer_space_to_read(&ptm->buffer) >= 1;
}

bool pty_master_can_write(file_t* file, size_t start)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    return tty_can_write(&ptm->pts->tty, file, start);
}

int pty_master_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    uint32_t leno = sync_ringbuffer_space_to_read(&ptm->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = sync_ringbuffer_read_user(&ptm->buffer, buf, leno);
    return leno;
}

int pty_master_write(file_t* file, void __user* buf, size_t start, size_t len)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    return tty_write(&ptm->pts->tty, file, buf, start, len);
}

int pty_master_fstat(file_t* file, stat_t* stat)
{
    dentry_t* dentry = file_dentry_assert(file);
    pty_master_entry_t* ptm = _ptm_get(dentry);
    ASSERT(ptm);

    stat->st_dev = MKDEV(128, INODE2PTSNO(dentry->inode_indx));
    return 0;
}

int pty_master_alloc(file_descriptor_t* fd)
{
    pty_master_entry_t* ptm = 0;
    for (int i = 0; i < PTYS_COUNT; i++) {
        if (pty_masters[i].dentry.inode_indx == 0) {
            /* According to dentry.c when inode_indx==0, dentry is free */
            ptm = &pty_masters[i];
            ptm->dentry.inode_indx = PTSNO2INODE(i);
            break;
        }
    }

    if (!ptm) {
        return -EBUSY;
    }

    // DENTRY_CUSTOM is set for the dentry, since it's not a cache of
    // a file (or a dir). We also set a required function for this
    // type of dentries free_inode, which is called when dentry is
    // freed.
    ptm->dentry.d_count = 1;
    ptm->dentry.flags = 0;
    dentry_set_flag(&ptm->dentry, DENTRY_CUSTOM);
    ptm->dentry.ops = &pty_master_ops;

    fd->file = file_init_pseudo_dentry(&ptm->dentry);
    fd->flags = O_RDWR;
    fd->offset = 0;
    dentry_put(&ptm->dentry);

    pty_slave_create(INODE2PTSNO(ptm->dentry.inode_indx), ptm);
    ptm->buffer = sync_ringbuffer_create_std();
    return 0;
}