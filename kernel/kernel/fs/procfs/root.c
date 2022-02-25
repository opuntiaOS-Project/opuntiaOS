/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/procfs/procfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <time/time_manager.h>

/**
 * inode: xxxxBBBBBBBBBBBBBBBBBBBBBBBBBBBB
 * x - level bits
 * B - bits of files at this level
 */
#define PROCFS_ROOT_LEVEL 1

extern const file_ops_t procfs_pid_ops;
extern const file_ops_t procfs_sys_ops;

static uint32_t procfs_root_sfiles_get_inode_index(int fileid);
static uint32_t procfs_root_self_get_inode_index(int fileid);

/* PID */
int procfs_root_getdents(dentry_t* dir, uint8_t* buf, off_t* offset, size_t len);
int procfs_root_lookup(dentry_t* dir, const char* name, size_t len, dentry_t** result);

/* FILES */
static bool procfs_root_uptime_can_read(dentry_t* dentry, size_t start);
static int procfs_root_uptime_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len);

static bool procfs_root_stat_can_read(dentry_t* dentry, size_t start);
static int procfs_root_stat_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len);

static bool procfs_root_meminfo_can_read(dentry_t* dentry, size_t start);
static int procfs_root_meminfo_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len);

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

const file_ops_t procfs_root_meminfo_ops = {
    .can_read = procfs_root_meminfo_can_read,
    .read = procfs_root_meminfo_read,
};

const file_ops_t procfs_root_stat_ops = {
    .can_read = procfs_root_stat_can_read,
    .read = procfs_root_stat_read,
};

static const procfs_files_t static_procfs_files[] = {
    { .name = "stat", .mode = S_IFREG | 0444, .ops = &procfs_root_stat_ops, .inode_index = procfs_root_sfiles_get_inode_index },
    { .name = "uptime", .mode = S_IFREG | 0444, .ops = &procfs_root_uptime_ops, .inode_index = procfs_root_sfiles_get_inode_index },
    { .name = "meminfo", .mode = S_IFREG | 0444, .ops = &procfs_root_meminfo_ops, .inode_index = procfs_root_sfiles_get_inode_index },
    { .name = "self", .mode = S_IFDIR | 0444, .ops = &procfs_pid_ops, .inode_index = procfs_root_self_get_inode_index },
    { .name = "sys", .mode = S_IFDIR | 0444, .ops = &procfs_sys_ops, .inode_index = procfs_root_self_get_inode_index },
};
#define PROCFS_STATIC_FILES_COUNT_AT_LEVEL (sizeof(static_procfs_files) / sizeof(procfs_files_t))

/**
 * HELPERS
 */

static uint32_t procfs_root_sfiles_get_inode_index(int fileid)
{
    return procfs_inode_get_index(PROCFS_ROOT_LEVEL, fileid);
}

static uint32_t procfs_root_self_get_inode_index(int fileid)
{
    return procfs_inode_get_index(PROCFS_ROOT_LEVEL, RUNNING_THREAD->process->pid + PROCFS_STATIC_FILES_COUNT_AT_LEVEL);
}

uint32_t procfs_root_get_pid_from_inode_index(uint32_t inode_index)
{
    return (inode_index & 0x0fffffff) - PROCFS_STATIC_FILES_COUNT_AT_LEVEL;
}

static uint32_t procfs_root_pid_get_inode_index(int procid)
{
    return procfs_inode_get_index(PROCFS_ROOT_LEVEL, procid + PROCFS_STATIC_FILES_COUNT_AT_LEVEL);
}

/**
 * ROOT
 */

int procfs_root_getdents(dentry_t* dir, uint8_t* buf, off_t* offset, size_t len)
{
    char name[8];
    int already_read = 0;
    dirent_t tmp;

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        if (*offset <= i) {
            ssize_t read = vfs_helper_write_dirent((dirent_t*)(buf + already_read), len, static_procfs_files[i].inode_index(i), static_procfs_files[i].name);
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
            uint32_t inode_index = procfs_root_pid_get_inode_index(proc[pidi].pid);
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

int procfs_root_lookup(dentry_t* dir, const char* name, size_t len, dentry_t** result)
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
                *result = dentry_get_no_inode(dir->dev_indx, static_procfs_files[i].inode_index(i), &newly_allocated);
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
                    *result = dentry_get_no_inode(dir->dev_indx, procfs_root_pid_get_inode_index(proc[pidi].pid), &newly_allocated);
                    if (newly_allocated) {
                        procfs_inode_t* new_procfs_inode = (procfs_inode_t*)((*result)->inode);
                        new_procfs_inode->mode = S_IFDIR | 0444;
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

static bool procfs_root_uptime_can_read(dentry_t* dentry, size_t start)
{
    return true;
}

static int procfs_root_uptime_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
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

static bool procfs_root_stat_can_read(dentry_t* dentry, size_t start)
{
    return true;
}

static int procfs_root_stat_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
    char res[128];
    int offset = 0;
    for (int i = 0; i < active_cpu_count(); i++) {
        time_t user = cpus[i].stat_user_ticks;
        time_t idle = cpus[i].idle_thread->stat_total_running_ticks;
        time_t system = cpus[i].stat_system_and_idle_ticks - idle;
        snprintf(res + offset, 128 - offset, "cpu%d %u %u %u %u\n", i, user, 0, system, idle);
        offset = strlen(res);
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

static bool procfs_root_meminfo_can_read(dentry_t* dentry, size_t start)
{
    return true;
}

static int procfs_root_meminfo_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
    char res[128];
    snprintf(res, 128, "MemTotal: %u kB\nMemFree: %u kB\n", pmm_get_ram_in_kb(), pmm_get_free_space_in_kb());
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