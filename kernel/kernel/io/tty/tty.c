/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/syscall_structs.h>
#include <mem/kmalloc.h>
#include <tasking/signal.h>
#include <tasking/tasking.h>

// #define TTY_DEBUG

int tty_init(tty_entry_t* tty)
{
    tty->line_count = 0;
    tty->buffer = sync_ringbuffer_create_std();
    if (!tty->buffer.ringbuffer.zone.start) {
        return -ENOMEM;
    }

    tty->termios.c_lflag |= ECHO | ICANON;
    return 0;
}

int tty_clear(tty_entry_t* tty)
{
    tty->line_count = 0;
    sync_ringbuffer_clear(&tty->buffer);
    return 0;
}

bool tty_can_read(tty_entry_t* tty, dentry_t* dentry, size_t start)
{
    if (TEST_FLAG(tty->termios.c_lflag, ICANON)) {
        return tty->line_count > 0;
    }
    return sync_ringbuffer_space_to_read(&tty->buffer) >= 1;
}

bool tty_can_write(tty_entry_t* tty, dentry_t* dentry, size_t start)
{
    return true;
}

int tty_read(tty_entry_t* tty, dentry_t* dentry, void __user* buf, size_t start, size_t len)
{
    size_t leno = sync_ringbuffer_space_to_read(&tty->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = sync_ringbuffer_read_user(&tty->buffer, buf, leno);
    if (TEST_FLAG(tty->termios.c_lflag, ICANON) && res == leno) {
        tty->line_count--;
    }
    return leno;
}

int tty_write(tty_entry_t* tty, dentry_t* dentry, void __user* buf, size_t start, size_t len)
{
    // TODO: Check line count correctly. Both read & write funcs.
    sync_ringbuffer_write_user(&tty->buffer, buf, len);
    tty->line_count++;
    return len;
}

int tty_ioctl(tty_entry_t* tty, dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    switch (cmd) {
    case TIOCGPGRP:
        return tty->pgid;
    case TIOCSPGRP:
        tty->pgid = arg;
        return 0;
    case TCGETS:
        umem_copy_to_user((void __user*)arg, (void*)&tty->termios, sizeof(termios_t));
        return 0;
    case TCSETS:
    case TCSETSW:
    case TCSETSF:
        umem_copy_from_user((void*)&tty->termios, (void __user*)arg, sizeof(termios_t));
        if (cmd == TCSETSF) {
            tty_clear(tty);
        }
        return 0;
    }

    return -EINVAL;
}
