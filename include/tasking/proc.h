/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__TASKING__PROC_H
#define __oneOS__X86__TASKING__PROC_H

#include <algo/dynamic_array.h>
#include <fs/vfs.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <tty/tty.h>
#include <types.h>
#include <x86/idt.h>

#define MAX_PROCESS_COUNT 1024
#define MAX_OPENED_FILES 16
#define SIGNALS_CNT 32

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} __attribute__((packed)) context_t;

enum PROC_ZONE_FLAGS {
    PROC_ZONE_EXEC = 0x1,
    PROC_ZONE_WRITE = 0x2,
};

enum PROC_ZONE_TYPES {
    PROC_ZONE_CODE = 0x1,
    PROC_ZONE_DATA = 0x2,
    PROC_ZONE_STACK = 0x4,
    PROC_ZONE_MMAP = 0x8,
};

struct proc_zone {
    uint32_t start;
    uint32_t len;
    uint32_t type;
    uint32_t flags;
};
typedef struct proc_zone proc_zone_t;

enum PROC_STATUS {
    PROC_INVALID = 0,
    PROC_RUNNING,
    PROC_DEAD,
    PROC_STOPPED,
    PROC_BLOCKED,
    PROC_DYING,
};

enum BLOCKER_REASON {
    BLOCKER_INVALID,
    BLOCKER_JOIN,
    BLOCKER_READ,
};

struct proc;
struct blocker {
    int reason;
    int (*should_unblock)(struct proc* p);
};
typedef struct blocker blocker_t;

struct proc {
    uint32_t sz;
    pdirectory_t* pdir;
    uint32_t pid;
    uint32_t status;

    zone_t kstack;
    context_t* context; // context of kernel's registers
    trapframe_t* tf;

    dynamic_array_t zones;

    dentry_t* cwd;
    file_descriptor_t* fds;
    tty_entry_t* tty;

    struct blocker blocker;
    int exit_code;
    struct proc* joinee;
    struct proc* joiner;
    dentry_t* blocker_dentry;

    uint32_t signals_mask;
    uint32_t pending_signals_mask;
    void* signal_handlers[SIGNALS_CNT];

    bool is_kthread;
};
typedef struct proc proc_t;

int init_join_blocker(struct proc* p);
int init_read_blocker(struct proc* proc, dentry_t* bd);

int proc_setup(proc_t* p);
int proc_setup_kstack(proc_t* p);
int proc_setup_tty(proc_t* p, tty_entry_t* tty);
void proc_segregs_setup(proc_t* p);
int proc_free(proc_t* p);

int kthread_setup(proc_t* p);
int kthread_setup_regs(proc_t* p, void* entry_point);
void kthread_segregs_setup(proc_t* p);

/**
 * PROC FD FUNCTIONS
 */

file_descriptor_t* proc_get_free_fd(proc_t* proc);
file_descriptor_t* proc_get_fd(proc_t* proc, uint32_t index);


/**
 * PROC ZONING
 */

proc_zone_t* proc_new_zone(proc_t* proc, uint32_t start, uint32_t len);
proc_zone_t* proc_new_random_zone(proc_t* proc, uint32_t len);
proc_zone_t* proc_new_random_zone_backward(proc_t* proc, uint32_t len);

#endif // __oneOS__X86__TASKING__PROC_H