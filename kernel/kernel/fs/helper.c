/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <tasking/tasking.h>

ssize_t vfs_helper_write_dirent(dirent_t* buf, uint32_t buf_len, uint32_t inode_index, const char* name)
{
    uint32_t name_len = strlen(name);
    int rec_len = 8 + name_len + 1;

    if (buf_len < rec_len) {
        return -ENOMEM;
    }

    buf->inode = inode_index;
    buf->rec_len = rec_len;
    buf->name_len = name_len;
    uint8_t* buf_u8 = (uint8_t*)buf;
    memcpy(buf_u8 + 8, name, name_len);
    buf_u8[rec_len - 1] = '\0';
    return rec_len;
}