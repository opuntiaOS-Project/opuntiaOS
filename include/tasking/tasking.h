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
#include <types.h>
#include <x86/idt.h>

#define MAX_PROCESS_COUNT 1024

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

    char* kstack;
    context_t* context; // context of kernel's registers
    trapframe_t* tf;

    dentry_t* cwd;
} __attribute__((packed)) proc_t;

proc_t proc[MAX_PROCESS_COUNT];
uint32_t nxt_proc;

proc_t* tasking_get_active_proc();

void switchuvm(proc_t* p);
void tasking_fork(trapframe_t* tf);
void tasking_exec(trapframe_t* tf);
void tasking_exit(trapframe_t* tf);

void tasking_start_init_proc();

#endif // __oneOS__X86__TASKING__TASKING_H