/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__TASKING__TASKING_H
#define __oneOS__X86__TASKING__TASKING_H

#include <fs/vfs.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
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

typedef struct {
    uint32_t sz;
    pdirectory_t* pdir;
    uint32_t pid;

    zone_t kstack;
    context_t* context; // context of kernel's registers
    trapframe_t* tf;

    dentry_t* cwd;
    file_descriptor_t* fds;

    uint32_t signals_mask;
    uint32_t pending_signals_mask;
    void* signal_handlers[SIGNALS_CNT];
} __attribute__((packed)) proc_t;

proc_t proc[MAX_PROCESS_COUNT];
uint32_t nxt_proc;

proc_t* tasking_get_active_proc();

/**
 * CPU FUNCTIONS
 */

void switchuvm(proc_t* p);

/**
 * TASK LOADING FUNCTIONS
 */

void tasking_start_init_proc();

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init();
file_descriptor_t* tasking_get_free_fd(proc_t* proc);
file_descriptor_t* tasking_get_fd(proc_t* proc, int index);

/**
 * SYSCALL IMPLEMENTATION
 */

void tasking_fork(trapframe_t* tf);
void tasking_exec(trapframe_t* tf);
void tasking_exit(trapframe_t* tf);

/**
 * SIGNALS
 */

void signal_init();

int signal_set_handler(proc_t* proc, int signo, void* handler);
int signal_set_allow(proc_t* proc, int signo);
int signal_set_private(proc_t* proc, int signo);
int signal_set_pending(proc_t* proc, int signo);

int signal_restore_proc_after_handling_signal(proc_t* proc);
int signal_dispatch_pending(proc_t* proc);

#endif // __oneOS__X86__TASKING__TASKING_H