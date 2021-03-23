/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <fs/vfs.h>
#include <libkern/kassert.h>
#include <libkern/log.h>
#include <libkern/mem.h>
#include <mem/kmalloc.h>
#include <platform/generic/system.h>
#include <syscalls/handlers.h>

// #define DENTRY_DEBUG

#define NOT_READ_INODE 0
#define READ_INODE 1
#define DENTRY_ALLOC_SIZE (4 * KB) /* Shows the size of list's parts. */
#define DENTRY_SWAP_THRESHOLD_FOR_INODE_CACHE (16 * KB)

extern vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
extern dynamic_array_t _vfs_fses;
extern uint32_t root_fs_dev_id;

static bool can_cache_inodes = 1; /* If we can cache inodes. */
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
        can_cache_inodes = 0;
    }
    return true;
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

static inline void dentry_delete_inode(dentry_t* dentry)
{
    ASSERT(dentry->d_count == 0 && dentry_test_flag(dentry, DENTRY_INODE_TO_BE_DELETED));
    dentry->ops->dentry.free_inode(dentry);
}

static inline void dentry_flush_inode(dentry_t* dentry)
{
    if (dentry_test_flag(dentry, DENTRY_DIRTY) && dentry->inode) {
        dentry->ops->dentry.write_inode(dentry);
        dentry_rem_flag(dentry, DENTRY_DIRTY);
    }
}

/**
 * dentry_delete_from_cache compeletly deletes dentry from cache.
 * In case when file was deleted and after that a new was created
 * with the same inode_id, dentry can't recognize and could use old
 * inode data. We need delete dentry from cache and free inode.
 */
static void dentry_delete_from_cache(dentry_t* dentry)
{
    /* This marks the dentry as deleted. */
    dentry->inode_indx = 0;
    if (dentry->inode) {
        kfree(dentry->inode);
    }
    stat_cached_inodes_area_size -= INODE_LEN;
    if (!need_to_free_inode_cache()) {
        can_cache_inodes = 1;
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
    if (!can_cache_inodes) {
        dentry_delete_from_cache(dentry);
    } else {
        if (need_to_free_inode_cache()) {
            free_inode_cache();
        }
    }
    stat_cached_dentries--;
}

static dentry_t* dentry_alloc_new(uint32_t dev_indx, uint32_t inode_indx, int need_to_read_inode)
{
    if (inode_indx == 0) {
        return 0;
    }

    dentry_t* dentry = dentry_cache_find_empty_entry();
    fs_desc_t* fs_desc;

    /* If inode_indx isn't 0, so we can say that we replace a valid dentry, which
       has area for storing inode allocated. */
    bool already_allocated_inode = (dentry->inode_indx != 0);
    dentry->d_count = 1;
    dentry->flags = 0;
    dentry->dev_indx = dev_indx;
    dentry->dev = &_vfs_devices[dentry->dev_indx];
    fs_desc = dynamic_array_get(&_vfs_fses, dentry->dev->fs);
    dentry->ops = fs_desc->ops;
    dentry->inode_indx = inode_indx;
    dentry->fsdata = dentry->ops->dentry.get_fsdata(dentry);
    dentry->parent = NULL;

    if (!already_allocated_inode) {
        dentry->inode = (inode_t*)kmalloc(INODE_LEN);
        stat_cached_inodes_area_size += INODE_LEN;
    }

    if (need_to_read_inode && dentry->ops->dentry.read_inode(dentry) < 0) {
        log_error("[Dentry] Can't read inode %d %d (dev, ino)", dev_indx, inode_indx);
        return 0;
    }

    stat_cached_dentries++;
    return dentry;
}

void dentry_set_inode(dentry_t* dentry, inode_t* inode)
{
    if (dentry->inode) {
        kfree(dentry->inode);
    }
    dentry->inode = inode;
}

void dentry_set_parent(dentry_t* to, dentry_t* parent)
{
    to->parent = dentry_duplicate(parent);
}

dentry_t* dentry_get_parent(dentry_t* dentry)
{
    return dentry->parent;
}

/**
 * Is a thread enrty point. The function flushes all inodes to drive.
 */
void dentry_flusher()
{
    for (;;) {
        log("WORK dentry_flusher");
        dentry_cache_list_t* dentry_cache_block = dentry_cache;
        while (dentry_cache_block) {
            int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
            for (int i = 0; i < dentries_in_block; i++) {
                if (dentry_cache_block->data[i].inode_indx != 0) {
                    system_disable_interrupts();
                    dentry_flush_inode(&dentry_cache_block->data[i]);
                    system_enable_interrupts();
                }
            }
            dentry_cache_block = dentry_cache_block->next;
        }
        ksys1(SYSSLEEP, 2);
    }
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
                if (!dentry_cache_block->data[i].d_count)
                    stat_cached_dentries++;
                return dentry_duplicate(&dentry_cache_block->data[i]);
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    /* It means no dentry in the cache. Let's add it. */
    return dentry_alloc_new(dev_indx, inode_indx, READ_INODE);
}

dentry_t* dentry_get_no_inode(uint32_t dev_indx, uint32_t inode_indx, int* newly_allocated)
{
    /* We try to find the dentry in the cache */
    dentry_cache_list_t* dentry_cache_block = dentry_cache;
    while (dentry_cache_block) {
        int dentries_in_block = dentry_cache_block->len / sizeof(dentry_t);
        for (int i = 0; i < dentries_in_block; i++) {
            if (dentry_cache_block->data[i].dev_indx == dev_indx && dentry_cache_block->data[i].inode_indx == inode_indx) {
                if (!dentry_cache_block->data[i].d_count)
                    stat_cached_dentries++;
                *newly_allocated = DENTRY_WAS_IN_CACHE;
                return dentry_duplicate(&dentry_cache_block->data[i]);
            }
        }
        dentry_cache_block = dentry_cache_block->next;
    }

    /* It means no dentry in the cache. Let's add it. */
    *newly_allocated = DENTRY_NEWLY_ALLOCATED;
    return dentry_alloc_new(dev_indx, inode_indx, NOT_READ_INODE);
}

dentry_t* dentry_duplicate(dentry_t* dentry)
{
    dentry->d_count++;
    return dentry;
}

static inline void dentry_put_impl(dentry_t* dentry)
{
    if (dentry->parent) {
        dentry_put(dentry->parent);
    }

    if (dentry_test_flag(dentry, DENTRY_CUSTOM)) {
        dentry->inode_indx = 0;
        if (dentry->ops->dentry.free_inode) {
            dentry->ops->dentry.free_inode(dentry);
        }
        return;
    }

    if (dentry_test_flag(dentry, DENTRY_INODE_TO_BE_DELETED)) {
#ifdef DENTRY_DEBUG
        log("Inode delete %d", dentry->inode_indx);
#endif
        dentry_delete_inode(dentry);
        dentry_delete_from_cache(dentry);
        return;
    }
#ifdef DENTRY_DEBUG
    log("Inode flushed %d", dentry->inode_indx);
#endif
    dentry_flush_inode(dentry);
    dentry_prefree(dentry);
}

void dentry_force_put(dentry_t* dentry)
{
    if (dentry_test_flag(dentry, DENTRY_MOUNTPOINT)) {
        return;
    }

    dentry->d_count = 0;
    dentry_put_impl(dentry);
}

void dentry_put(dentry_t* dentry)
{
    ASSERT(dentry->d_count > 0);
    dentry->d_count--;

    if (dentry->d_count == 0) {
        dentry_put_impl(dentry);
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