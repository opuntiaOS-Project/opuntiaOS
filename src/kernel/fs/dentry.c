/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <drivers/display.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <utils/mem.h>

#define DENTRY_ALLOC_SIZE (4 * KB) /* Shows the size of list's parts. */
#define DENTRY_SWAP_THRESHOLD_FOR_INODE_CACHE (16 * KB)

extern vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
extern dynamic_array_t _vfs_fses;
extern uint32_t root_fs_dev_id;

static bool can_stay_inode_cache = 1; /* If we can stay inode cache. */
static uint32_t stat_cached_dentries = 0; /* Count of dentries which are held. */
static uint32_t stat_cached_inodes_area_size = 0; /* Sum of all areas which is used for holding inodes. */
static dentry_cache_list_t* dentry_cache;
static uint16_t* dentry_cahced;

static inline bool need_to_free_inode_cache()
{
    return (stat_cached_inodes_area_size > DENTRY_SWAP_THRESHOLD_FOR_INODE_CACHE);
}

static inline bool free_inode_cache()
{
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    dentry_t* valid_dentry_candidate = 0;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].d_count == 0) {
                dentry_cache_block->data[i].inode_indx = 0;
                stat_cached_inodes_area_size -= INODE_LEN;
                kfree(dentry_cache_block->data[i].inode);
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    if (need_to_free_inode_cache()) {
        can_stay_inode_cache = 0;
    }
}

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

/**
 * In this function, we try to find an entry to fill it with a new dentry.
 * Since we have a valid dentries in the cache, which isn't held by someone, they are
 * also condidates to be replaced. But because we want to have more valid entries in the
 * cache, we will start to look for a complitely free entry.
 */
static dentry_t* dentry_cache_find_empty_entry()
{
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    dentry_t* valid_dentry_candidate = 0;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].inode_indx == 0) {
                return &dentry_cache_block->data[i];
            }
            if (dentry_cache_block->data[i].d_count == 0) {
                valid_dentry_candidate = &dentry_cache_block->data[i];
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    /* If we hasn't found a complitely free entry, let's erase a 
       valid dentries in the cache, which isn't held by someone. */
    if (valid_dentry_candidate) {
        return valid_dentry_candidate;
    }

    /* If there is no space, let's allocate a bigger area. */
    dentry_cache_alloc();
    dentry_cache_list_t* last = dentry_cache;
    while (last->next != 0) {
        last = last->next;
    }
    return &last->data[0];
}

static inline void dentry_flush_inode(dentry_t* dentry)
{
    if (dentry_test_flag(dentry, DENTRY_DIRTY)) {
        dentry->ops->dentry.write_inode(dentry);
    }
}

/**
 * dentry_prefree puts the dentry in a state when it can be
 * safely replaced with a new one.
 * Note: the dentry stays valid and can be reused without needless to
 *       read all data again.
 */
static void dentry_prefree(dentry_t* dentry)
{
    /* This marks the dentry as deleted. */
    if (!can_stay_inode_cache) {
        dentry->inode_indx = 0;
        kfree(dentry->inode);
        stat_cached_inodes_area_size -= INODE_LEN;
        if (!need_to_free_inode_cache()) {
            can_stay_inode_cache = 1;
        }
    } else {
        if (need_to_free_inode_cache()) {
            free_inode_cache();
        }
    }
    stat_cached_dentries--;
}

static dentry_t* dentry_alloc_new(uint32_t dev_indx, uint32_t inode_indx)
{
    dentry_t* dentry = dentry_cache_find_empty_entry();

    /* If inode_indx isn't 0, so we can say that we replace a valid dentry, which
       has area for storing inode allocated. */
    bool already_allocated_inode = (dentry->inode_indx != 0);

    dentry->d_count = 1;
    dentry->flags = 0;
    dentry->dev_indx = dev_indx;
    dentry->dev = &_vfs_devices[dentry->dev_indx];
    dentry->ops = dynamic_array_get(&_vfs_fses, dentry->dev->fs);
    dentry->inode_indx = inode_indx;
    dentry->fsdata = dentry->ops->dentry.get_fsdata(dentry);
    
    if (!already_allocated_inode) {
        dentry->inode = (inode_t*)kmalloc(INODE_LEN);
        stat_cached_inodes_area_size += INODE_LEN;
    }

    if (dentry->ops->dentry.read_inode(dentry) < 0) {
        kprintf("CANT READ INODE");
        return 0;
    }

    stat_cached_dentries++;
    return dentry;
}

void dentry_set_parent(dentry_t* to, dentry_t* parent)
{
    to->parent_inode_indx = parent->inode_indx;
    to->parent_dev_indx = parent->dev_indx;
}

dentry_t* dentry_get_parent(dentry_t* dentry)
{
    return dentry_get(dentry->parent_dev_indx, dentry->parent_inode_indx);
}

/**
 * There are 3 cases for each entry in a cache array:
 * 1) We have a valid dentry which is held by someone.
 * 2) We have a valid dentry which isn'y held by someone and ready to swapped out.
 * 3) We have an unsed entry in the cache array.
 */
dentry_t* dentry_get(uint32_t dev_indx, uint32_t inode_indx)
{
    /* We try to find the dentry in the cache */
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].dev_indx == dev_indx && dentry_cache_block->data[i].inode_indx == inode_indx) {
                if (!dentry_cache_block->data[i].d_count) stat_cached_dentries++;
                return dentry_duplicate(&dentry_cache_block->data[i]);
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    /* It means no dentry in the cache. Let's add it. */
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
    dentry_flush_inode(dentry);
    dentry_prefree(dentry);
}

void dentry_put(dentry_t* dentry)
{
    dentry->d_count--;

    if (dentry->d_count == 0) {
        dentry_flush_inode(dentry);
        dentry_prefree(dentry);
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

inline void dentry_inode_set_flag(dentry_t* dentry, mode_t mode)
{
    if (!dentry_inode_test_flag(dentry, mode)) {
        dentry_set_flag(dentry, DENTRY_DIRTY);
    }
    dentry->inode->mode |= mode;
}

inline bool dentry_inode_test_flag(dentry_t* dentry, mode_t mode)
{
    return (dentry->inode->mode & mode) > 0;
}

inline void dentry_inode_rem_flag(dentry_t* dentry, mode_t mode)
{
    if (dentry_inode_test_flag(dentry, mode)) {
        dentry_set_flag(dentry, DENTRY_DIRTY);
    }
    dentry->inode->mode &= ~mode;
}

uint32_t dentry_stat_cached_count()
{
    return stat_cached_dentries;
}