/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/display.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <utils/mem.h>

#define DENTRY_ALLOC_SIZE 4096 // 4KB. Shows the size of list's parts

extern vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
extern fs_ops_t _vfs_fses[MAX_DRIVERS_COUNT];
extern uint8_t _vfs_fses_count;
extern uint32_t root_fs_dev_id;

static dentry_cache_list_t* dentry_cache;
static uint16_t* dentry_cahced;

static void dentry_cache_alloc()
{
    dentry_cache_list_t* list_block = (dentry_cache_list_t*)kmalloc(DENTRY_ALLOC_SIZE);
    memset((uint8_t*)list_block, 0, DENTRY_ALLOC_SIZE);
    list_block->data = (dentry_t*)&list_block[1];
    list_block->len = DENTRY_ALLOC_SIZE - ((uint32_t)&list_block[1] - (uint32_t)&list_block[0]);

    if (dentry_cache == 0) {
        dentry_cache = list_block;
    } else {
        dentry_cache_list_t* last = dentry_cache;
        while (last->next != 0) {
            last = last->next;
        }
        last->next = list_block;
        list_block->prev = last;
    }
}

static dentry_t* dentry_cache_find_empty()
{
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].inode_indx == 0) {
                return &dentry_cache_block->data[i];
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }
    dentry_cache_alloc();
    dentry_cache_list_t* last = dentry_cache;
    while (last->next != 0) {
        last = last->next;
    }
    return &last->data[0];
}

static void dentry_flush_inode(dentry_t* dentry)
{
    if (dentry_test_flag(dentry, DENTRY_DIRTY)) {
        dentry->ops->dentry.write_inode(dentry);
    }

    // This marks the dentry as deleted.
    dentry->inode_indx = 0;
    kfree(dentry->inode);
}

static dentry_t* dentry_alloc_new(uint32_t dev_indx, uint32_t inode_indx)
{
    dentry_t* dentry = dentry_cache_find_empty();

    dentry->d_count = 1;
    dentry->flags = 0;
    dentry->dev_indx = dev_indx;
    dentry->dev = &_vfs_devices[dentry->dev_indx];
    dentry->ops = &_vfs_fses[dentry->dev->fs];
    dentry->inode_indx = inode_indx;
    dentry->fsdata = dentry->ops->dentry.get_fsdata(dentry);
    dentry->inode = (inode_t*)kmalloc(INODE_LEN);
    if (dentry->ops->dentry.read_inode(dentry) < 0) {
        kprintf("CANT READ INODE");
    }
    return dentry;
}

void dentry_set_parent(dentry_t* to, dentry_t* parent)
{
    to->parent_inode_indx = parent->inode_indx;
    to->parent_dev_indx = parent->dev_indx;
}

dentry_t* dentry_get(uint32_t dev_indx, uint32_t inode_indx)
{
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].dev_indx == dev_indx && dentry_cache_block->data[i].inode_indx == inode_indx) {
                return &dentry_cache_block->data[i];
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    // Means no dentry in cache. Let's read and add.
    return dentry_alloc_new(dev_indx, inode_indx);
}

dentry_t* dentry_duplicate(dentry_t* dentry)
{
    dentry->d_count++;
    return dentry;
}

void dentry_force_put(dentry_t* dentry)
{
    if (dentry_test_flag(dentry, DENTRY_MOUNTPOINT)) {
        return;
    }

    dentry->d_count = 0;
    if (dentry->d_count == 0) {
        dentry_flush_inode(dentry);
    }
}

void dentry_put(dentry_t* dentry)
{
    dentry->d_count--;

    if (dentry->d_count == 0) {
        dentry_flush_inode(dentry);
    }
}

void dentry_put_all_dentries_of_dev(uint32_t dev_indx)
{
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].dev_indx == dev_indx && dentry_cache_block->data[i].inode != 0) {
                dentry_force_put(&dentry_cache_block->data[i]);
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }
}

inline void dentry_set_flag(dentry_t* dentry, uint32_t flag)
{
    dentry->flags |= flag;
}

inline bool dentry_test_flag(dentry_t* dentry, uint32_t flag)
{
    return (dentry->flags & flag) > 0;
}

inline void dentry_rem_flag(dentry_t* dentry, uint32_t flag)
{
    dentry->flags &= ~flag;
}

inline void dentry_inode_set_flag(dentry_t* dentry, uint32_t flag)
{
    if (!dentry_inode_test_flag(dentry, flag)) {
        dentry_set_flag(dentry, DENTRY_DIRTY);
    }
    dentry->inode->mode |= flag;
    
}

inline bool dentry_inode_test_flag(dentry_t* dentry, uint32_t flag)
{
    return (dentry->inode->mode & flag) > 0;
}

inline void dentry_inode_rem_flag(dentry_t* dentry, uint32_t flag)
{
    if (dentry_inode_test_flag(dentry, flag)) {
        dentry_set_flag(dentry, DENTRY_DIRTY);
    }
    dentry->inode->mode &= ~flag;
}