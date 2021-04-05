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
#include <mem/kmalloc.h>
#include <tasking/tasking.h>

#define NORM_FILENAME(x) ((x + 0x3) & (uint32_t)(~0b11))

ssize_t vfs_helper_write_dirent(dirent_t* buf, uint32_t buf_len, uint32_t inode_index, const char* name)
{
    uint32_t name_len = strlen(name);
    int phys_rec_len = 8 + NORM_FILENAME(name_len) + 1;
    int virt_rec_len = 8 + name_len + 1;

    if (buf_len < phys_rec_len) {
        return -ENOMEM;
    }

    buf->inode = inode_index;
    buf->rec_len = phys_rec_len;
    buf->name_len = name_len;
    uint8_t* buf_u8 = (uint8_t*)buf;
    memcpy(buf_u8 + 8, name, name_len);
    buf_u8[virt_rec_len - 1] = '\0';
    return phys_rec_len;
}

/**
 * The function splits the input path/to/the/file to 2 part parts:
 * path/to/the and file. 
 * Note that the function shorts the input string to be the path/to/the
 * and allocates a new one which contains file path.
 */
char* vfs_helper_split_path_with_name(char* path, size_t len)
{
    char* name = NULL;
    if (!path) {
        return NULL;
    }

    if (!len) {
        return NULL;
    }

    size_t len_of_name = 0;
    char* res = &path[len - 1];

    while (*res == '/') {
        res--;
    }

    while (res >= path) {
        if (*res == '/') {
            res++;
            goto alloc;
        }
        len_of_name++;
        res--;
    }
    res++;

alloc:
    name = kmalloc(len_of_name + 1);
    memcpy(name, res, len_of_name);
    *res = '\0';
    name[len_of_name] = '\0';
    return name;
}

/**
 * The function restores the path after it was splitted with a help of
 * vfs_helper_split_path_with_name. 
 * Note that the function must be called only after vfs_helper_split_path_with_name.
 */
void vfs_helper_restore_full_path_after_split(char* path, char* name)
{
    size_t path_len = strlen(path);
    path[path_len] = name[0];
}