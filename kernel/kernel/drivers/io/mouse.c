/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/io/mouse.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/libkern.h>

static ringbuffer_t gmouse_buffer;

static bool _generic_mouse_can_read(file_t* file, size_t start)
{
    return ringbuffer_space_to_read(&gmouse_buffer) >= 1;
}

static int _generic_mouse_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    size_t read_len;

    read_len = ringbuffer_space_to_read(&gmouse_buffer);
    if (read_len > len)
        read_len = len;

    ringbuffer_read_user(&gmouse_buffer, buf, read_len);
    return read_len;
}

int generic_mouse_create_devfs()
{
    path_t vfspth;
    if (vfs_resolve_path("/dev", &vfspth) < 0)
        return -1;

    file_ops_t fops = { 0 };
    fops.can_read = _generic_mouse_can_read;
    fops.read = _generic_mouse_read;
    devfs_inode_t* res = devfs_register(&vfspth, MKDEV(10, 1), "mouse", 5, S_IFCHR | 0400, &fops);

    path_put(&vfspth);
    return 0;
}

void generic_mouse_init()
{
    gmouse_buffer = ringbuffer_create_std();
}

void generic_mouse_send_packet(mouse_packet_t* packet)
{
    ringbuffer_write(&gmouse_buffer, (uint8_t*)packet, sizeof(mouse_packet_t));
}