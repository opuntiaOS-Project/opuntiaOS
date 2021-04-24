/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/procfs/procfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <tasking/tasking.h>
#include <time/time_manager.h>

/**
 * inode: xxxxBBBBBBBBBBBBBBBBBBBBBBBBBBBB 
 * x - level bits
 * B - bits of files at this level
 */
#define PROCFS_ROOT_LEVEL 1

extern const file_ops_t procfs_pid_ops;

/* PID */
int procfs_root_getdents(dentry_t* dir, uint8_t* buf, uint32_t* offset, uint32_t len);
int procfs_root_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result);

/* FILES */
static bool procfs_root_uptime_can_read(dentry_t* dentry, uint32_t start);
static int procfs_root_uptime_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
static bool procfs_root_stat_can_read(dentry_t* dentry, uint32_t start);
static int procfs_root_stat_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);

/**
 * DATA
 */

const file_ops_t procfs_root_ops = {
    .getdents = procfs_root_getdents,
    .lookup = procfs_root_lookup,
};

const file_ops_t procfs_root_uptime_ops = {
    .can_read = procfs_root_uptime_can_read,
    .read = procfs_root_uptime_read,
};

const file_ops_t procfs_root_stat_ops = {
    .can_read = procfs_root_stat_can_read,
    .read = procfs_root_stat_read,
};

static const procfs_files_t static_procfs_files[] = {
    { .name = "stat", .mode = 0, .ops = &procfs_root_stat_ops },
    { .name = "uptime", .mode = 0, .ops = &procfs_root_uptime_ops },
};
#define PROCFS_STATIC_FILES_COUNT_AT_LEVEL (sizeof(static_procfs_files) / sizeof(procfs_files_t))

/**
 * HELPERS
 */

static uint32_t procfs_root_sfiles_get_inode_index(int fileid)
{
    return procfs_get_inode_index(PROCFS_ROOT_LEVEL, fileid);
}

uint32_t procfs_root_get_pid_from_inode_index(uint32_t inode_index)
{
    return (inode_index & 0x0fffffff) - PROCFS_STATIC_FILES_COUNT_AT_LEVEL;
}

static uint32_t procfs_root_pid_get_inode_index(int procid)
{
    return procfs_get_inode_index(PROCFS_ROOT_LEVEL, procid + PROCFS_STATIC_FILES_COUNT_AT_LEVEL);
}

/**
 * ROOT
 */

int procfs_root_getdents(dentry_t* dir, uint8_t* buf, uint32_t* offset, uint32_t len)
{
    char name[8];
    int already_read = 0;
    dirent_t tmp;

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        if (*offset <= i) {
            uint32_t inode_index = procfs_root_sfiles_get_inode_index(i);
            ssize_t read = vfs_helper_write_dirent((dirent_t*)(buf + already_read), len, inode_index, static_procfs_files[i].name);
            if (read <= 0) {
                if (!already_read) {
                    return -EINVAL;
                }
                return already_read;
            }
            already_read += read;
            len -= read;
            (*offset)++;
        }
    }

    // Pids
    int pidi = *offset - PROCFS_STATIC_FILES_COUNT_AT_LEVEL;
    for (; pidi < MAX_PROCESS_COUNT; pidi++) {
        if (proc[pidi].status == PROC_ALIVE) {
            snprintf(name, 8, "%d", proc[pidi].pid);
            uint32_t inode_index = procfs_root_pid_get_inode_index(pidi);
            ssize_t read = vfs_helper_write_dirent((dirent_t*)(buf + already_read), len, inode_index, name);
            if (read <= 0) {
                if (!already_read) {
                    return -EINVAL;
                }
                return already_read;
            }
            already_read += read;
            len -= read;
        }
        (*offset) = PROCFS_STATIC_FILES_COUNT_AT_LEVEL + pidi;
    }

    return already_read;
}

int procfs_root_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result)
{
    char pid_name[8];
    procfs_inode_t* procfs_inode = (procfs_inode_t*)dir->inode;

    if (len == 1) {
        if (name[0] == '.') {
            *result = dentry_duplicate(dir);
            return 0;
        }
    }

    if (len == 2) {
        if (name[0] == '.' && name[1] == '.') {
            *result = dentry_duplicate(dir->parent);
            return 0;
        }
    }

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        uint32_t child_name_len = strlen(static_procfs_files[i].name);
        if (len == child_name_len) {
            if (strncmp(name, static_procfs_files[i].name, len) == 0) {
                int newly_allocated;
                *result = dentry_get_no_inode(dir->dev_indx, procfs_root_sfiles_get_inode_index(i), &newly_allocated);
                if (newly_allocated) {
                    procfs_inode_t* new_procfs_inode = (procfs_inode_t*)((*result)->inode);
                    new_procfs_inode->mode = static_procfs_files[i].mode;
                    new_procfs_inode->ops = static_procfs_files[i].ops;
                }
                return 0;
            }
        }
    }

    // Pids
    for (int pidi = 0; pidi < MAX_PROCESS_COUNT; pidi++) {
        if (proc[pidi].status == PROC_ALIVE) {
            snprintf(pid_name, 8, "%d", proc[pidi].pid);
            uint32_t child_name_len = strlen(pid_name);
            if (len == child_name_len) {
                if (strncmp(name, pid_name, len) == 0) {
                    int newly_allocated;
                    *result = dentry_get_no_inode(dir->dev_indx, procfs_root_pid_get_inode_index(pidi), &newly_allocated);
                    if (newly_allocated) {
                        procfs_inode_t* new_procfs_inode = (procfs_inode_t*)((*result)->inode);
                        new_procfs_inode->mode = S_IFDIR;
                        new_procfs_inode->ops = &procfs_pid_ops;
                    }
                    return 0;
                }
            }
        }
    }

    return -ENOENT;
}

/**
 * FILES
 */

static bool procfs_root_uptime_can_read(dentry_t* dentry, uint32_t start)
{
    return true;
}

static int procfs_root_uptime_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    char res[16];
    snprintf(res, 16, "%d", timeman_seconds_since_boot());
    size_t size = strlen(res);

    if (start == size) {
        return 0;
    }

    if (len < size) {
        return -EFAULT;
    }

    memcpy(buf, res, size);
    return size;
}

static bool procfs_root_stat_can_read(dentry_t* dentry, uint32_t start)
{
    return true;
}

static int procfs_root_stat_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    char res[64];
    for (int i = 0; i < CPU_CNT; i++) {
        time_t user = cpus[i].stat_user_ticks;
        time_t idle = cpus[i].idle_thread->stat_total_running_ticks;
        time_t system = cpus[i].stat_system_and_idle_ticks - idle;
        snprintf(res, 64, "cpu%d %u %u %u %u", i, user, 0, system, idle);
    }
    size_t size = strlen(res);

    if (start == size) {
        return 0;
    }

    if (len < size) {
        return -EFAULT;
    }

    memcpy(buf, res, size);
    return size;
}