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
#include <tasking/proc.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <types.h>
#include <x86/idt.h>

#define MAX_PROCESS_COUNT 1024
#define MAX_OPENED_FILES 16
#define SIGNALS_CNT 32

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