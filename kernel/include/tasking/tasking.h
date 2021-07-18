/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_TASKING_H
#define _KERNEL_TASKING_TASKING_H

#include <drivers/generic/fpu.h>
#include <fs/vfs.h>
#include <libkern/types.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <platform/generic/tasking/context.h>
#include <platform/generic/tasking/trapframe.h>
#include <tasking/cpu.h>
#include <tasking/proc.h>
#include <tasking/thread.h>

#define MAX_PROCESS_COUNT 1024
#define MAX_DYING_PROCESS_COUNT 8
#define MAX_OPENED_FILES 16
#define SIGNALS_CNT 32

extern proc_t proc[MAX_PROCESS_COUNT];
extern uint32_t nxt_proc;
extern uint32_t ended_proc;

proc_t* tasking_get_proc(uint32_t pid);
proc_t* tasking_get_proc_by_pdir(pdirectory_t* pdir);

/**
 * CPU FUNCTIONS
 */

void switchuvm(thread_t* p);

/**
 * TASK LOADING FUNCTIONS
 */

void tasking_start_init_proc();
proc_t* tasking_create_kernel_thread(void* entry_point, void* data);

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init();
void tasking_kill_dying();

/**
 * SYSCALL IMPLEMENTATION
 */

void tasking_fork(trapframe_t* tf);
int tasking_exec(const char* path, const char** argv, const char** env);
void tasking_exit(int exit_code);
int tasking_waitpid(int pid);
int tasking_kill(thread_t* thread, int signo);

/**
 * SIGNALS
 */

void signal_init();

int signal_set_handler(thread_t* thread, int signo, void* handler);
int signal_set_allow(thread_t* thread, int signo);
int signal_set_private(thread_t* thread, int signo);
int signal_set_pending(thread_t* thread, int signo);
int signal_rem_pending(thread_t* thread, int signo);

int signal_restore_thread_after_handling_signal(thread_t* thread);
int signal_dispatch_pending(thread_t* thread);

#endif // _KERNEL_TASKING_TASKING_H