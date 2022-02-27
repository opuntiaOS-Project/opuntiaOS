/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_IO_TTY_VCONSOLE_H
#define _KERNEL_IO_TTY_VCONSOLE_H

#include <algo/sync_ringbuffer.h>
#include <io/tty/tty.h>
#include <libkern/types.h>

#define VCONSOLE_MAX_COUNT 8
#define VCONSOLE_BUFFER_SIZE 1024

struct vconsole_entry {
    int id;
    int inode_indx;
    tty_entry_t tty;
};
typedef struct vconsole_entry vconsole_entry_t;

extern vconsole_entry_t vconsoles[VCONSOLE_MAX_COUNT];

vconsole_entry_t* vconsole_new();
void vconsole_eat_key(int key);

#endif // _KERNEL_IO_TTY_TTY_H