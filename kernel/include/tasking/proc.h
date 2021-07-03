/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_PROC_H
#define _KERNEL_TASKING_PROC_H

#include <algo/dynamic_array.h>
#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <libkern/atomic.h>
#include <libkern/lock.h>
#include <libkern/types.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

#define MAX_PROCESS_COUNT 1024
#define MAX_OPENED_FILES 16

struct blocker;

/* Like Page Flags in vmm.h */
enum ZONE_FLAGS {
    ZONE_WRITABLE = 0x1,
    ZONE_READABLE = 0x2,
    ZONE_EXECUTABLE = 0x4,
    ZONE_NOT_CACHEABLE = 0x8,
    ZONE_COW = 0x10,
    ZONE_USER = 0x20,
};

enum ZONE_TYPES {
    ZONE_TYPE_NULL = 0x0,
    ZONE_TYPE_CODE = 0x1,
    ZONE_TYPE_DATA = 0x2,
    ZONE_TYPE_STACK = 0x4,
    ZONE_TYPE_BSS = 0x8,
    ZONE_TYPE_DEVICE = 0x10,
    ZONE_TYPE_MAPPED = 0x20,
    ZONE_TYPE_MAPPED_FILE_PRIVATLY = 0x40,
    ZONE_TYPE_MAPPED_FILE_SHAREDLY = 0x80,
};

struct proc_zone {
    uint32_t start;
    uint32_t len;
    uint32_t type;
    uint32_t flags;
    dentry_t* file;
    uint32_t offset;
};
typedef struct proc_zone proc_zone_t;

enum PROC_STATUS {
    PROC_INVALID = 0,
    PROC_ALIVE,
    PROC_DEAD,
    PROC_DYING,
};

struct thread;
struct proc {
    pdirectory_t* pdir;
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    uint32_t prio;
    uint32_t status;
    struct thread* main_thread;
    lock_t lock;

    uid_t uid;
    gid_t gid;
    uid_t euid;
    gid_t egid;
    uid_t suid;
    gid_t sgid;

    dynamic_array_t zones;

    dentry_t* proc_file;
    dentry_t* cwd;
    file_descriptor_t* fds;
    tty_entry_t* tty;

    bool is_kthread;
};
typedef struct proc proc_t;

/**
 * PROC FUNCTIONS
 */
uint32_t proc_alloc_pid();
struct thread* thread_by_pid(uint32_t pid);

int proc_init_storage();
int proc_setup(proc_t* p);
int proc_setup_with_uid(proc_t* p, uid_t uid, gid_t gid);
int proc_setup_tty(proc_t* p, tty_entry_t* tty);
int proc_fill_up_stack(proc_t* p, int argc, char** argv, char** env);
int proc_free(proc_t* p);

struct thread* proc_alloc_thread();
struct thread* proc_create_thread(proc_t* p);
void proc_kill_all_threads(proc_t* p);
void proc_kill_all_threads_except(proc_t* p, struct thread* gthread);

/**
 * KTHREAD FUNCTIONS
 */

int kthread_setup(proc_t* p);
int kthread_setup_regs(proc_t* p, void* entry_point);
void kthread_setup_segment_regs(proc_t* p);
int kthread_fill_up_stack(struct thread* thread, void* data);
int kthread_free(proc_t* p);

int proc_load(proc_t* p, struct thread* main_thread, const char* path);
int proc_copy_of(proc_t* new_proc, struct thread* from_thread);

int proc_die(proc_t* p);
int proc_block_all_threads(proc_t* p, struct blocker* blocker);

/**
 * PROC FD FUNCTIONS
 */

int proc_chdir(proc_t* p, const char* path);
file_descriptor_t* proc_get_free_fd(proc_t* p);
file_descriptor_t* proc_get_fd(proc_t* p, uint32_t index);
int proc_get_fd_id(proc_t* proc, file_descriptor_t* fd);

/**
 * PROC ZONER FUNCTIONS
 */

proc_zone_t* proc_new_zone(proc_t* p, uint32_t start, uint32_t len);
proc_zone_t* proc_extend_zone(proc_t* proc, uint32_t start, uint32_t len);
proc_zone_t* proc_new_random_zone(proc_t* p, uint32_t len);
proc_zone_t* proc_new_random_zone_backward(proc_t* p, uint32_t len);
proc_zone_t* proc_find_zone(proc_t* p, uint32_t addr);
proc_zone_t* proc_find_zone_no_proc(dynamic_array_t* zones, uint32_t addr);
int proc_delete_zone_no_proc(dynamic_array_t*, proc_zone_t*);
int proc_delete_zone(proc_t*, proc_zone_t*);

#endif // _KERNEL_TASKING_PROC_H