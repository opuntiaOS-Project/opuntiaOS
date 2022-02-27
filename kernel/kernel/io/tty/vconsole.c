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
#include <io/tty/vconsole.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/syscall_structs.h>
#include <mem/kmalloc.h>
#include <tasking/signal.h>
#include <tasking/tasking.h>

// #define VCONSOLE_DEBUG

static int next_vconsole = 0;
static vconsole_entry_t* active_vconsole = 0;
vconsole_entry_t vconsoles[TTY_MAX_COUNT];

static vconsole_entry_t* _vconsole_get(dentry_t* dentry)
{
    for (int i = 0; i < TTY_MAX_COUNT; i++) {
        if (dentry->inode_indx == vconsoles[i].inode_indx) {
            return &vconsoles[i];
        }
    }
    return 0;
}

inline static vconsole_entry_t* _vconsole_active()
{
    return active_vconsole;
}

bool vconsole_can_read(dentry_t* dentry, size_t start)
{
    vconsole_entry_t* vconsole = _vconsole_get(dentry);
    return tty_can_read(&vconsole->tty, dentry, start);
}

bool vconsole_can_write(dentry_t* dentry, size_t start)
{
    vconsole_entry_t* vconsole = _vconsole_get(dentry);
    return tty_can_write(&vconsole->tty, dentry, start);
}

int vconsole_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
    return 0;
}

int _vconsole_process_esc_seq(uint8_t* buf)
{
    if (buf[0] != '\x1b') {
        return 0;
    }
    int argv[4] = { 0, 0, 0, 0 };
    int id = 1;
    int argc = 0;

    if (buf[id] == '[') {
        id++;
        for (;;) {
            int len = 0;
            while ('0' <= buf[id + len] && buf[id + len] <= '9') {
                len++;
            }
            argv[argc++] = stoi(&buf[id], len);
            id += len;
            if (buf[id] != ';') {
                break;
            }
            id++;
        }
    }

    char cmd = buf[id];
    id++;
    switch (cmd) {
    case 'J':
        if (argv[0] == 2) {
            // clean_screen();
        }
        return id;
    case 'H':
        if (argc == 0) {
            // set_cursor_offset(get_offset(0, 0));
        }
        if (argc == 2) {
            // set_cursor_offset(get_offset(argv[1] - 1, argv[0] - 1));
        }
        return id;
    }
    return 0;
}

int vconsole_write(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
#ifdef VCONSOLE_DEBUG
    time_t cur_time = timeman_now();
    int secs = cur_time % 60;
    cur_time /= 60;
    int mins = cur_time % 60;
    cur_time /= 60;
    int hrs = cur_time % 24;
    time_t ticks = timeman_get_ticks_from_last_second();
    log_not_formatted("[%d:%d:%d.%d] ", hrs, mins, secs, ticks);
#endif
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\x1b') {
            i += _vconsole_process_esc_seq(&buf[i]);
        } else {
            log_not_formatted("%c", buf[i]);
            // print_char(buf[i], WHITE_ON_BLACK, -1, -1);
        }
    }

    return len;
}

int vconsole_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    vconsole_entry_t* vconsole = _vconsole_get(dentry);
    return tty_ioctl(&vconsole->tty, dentry, cmd, arg);
}

vconsole_entry_t* vconsole_new()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return 0;
    }

    char* name = "tty ";
    name[3] = next_vconsole + '0';
    file_ops_t fops = { 0 };
    fops.can_read = vconsole_can_read;
    fops.can_write = vconsole_can_write;
    fops.read = vconsole_read;
    fops.write = vconsole_write;
    fops.ioctl = vconsole_ioctl;
    devfs_inode_t* res = devfs_register(mp, MKDEV(4, next_vconsole), name, 4, S_IFCHR | 0777, &fops);
    vconsoles[next_vconsole].id = next_vconsole;
    vconsoles[next_vconsole].inode_indx = res->index;

    tty_init(&vconsoles[next_vconsole].tty);
    if (!vconsoles[next_vconsole].tty.buffer.ringbuffer.zone.start) {
        log_error("Error: tty buffer allocation");
        while (1) { }
    }
    next_vconsole++;

    dentry_put(mp);
    return &vconsoles[next_vconsole - 1];
}

static void _vconsole_echo_key(tty_entry_t* tty, int key)
{
    if (tty->termios.c_lflag & ECHO) {
        // print_char(key, WHITE_ON_BLACK, -1, -1);
    }
}