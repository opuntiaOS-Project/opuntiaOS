/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_TASKING_H
#define _KERNEL_TASKING_TASKING_H

#include <fs/vfs.h>
#include <libkern/types.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/generic/fpu/fpu.h>
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

proc_t* tasking_get_proc(pid_t pid);

/**
 * CPU FUNCTIONS
 */

void switchuvm(thread_t* p);

/**
 * TASK LOADING FUNCTIONS
 */

void tasking_start_init_proc();
proc_t* tasking_create_kernel_thread(void* entry_point, void* data);
proc_t* tasking_run_kernel_thread(void* entry_point, void* data);

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init();
void tasking_kill_dying();
bool tasking_should_become_zombie(proc_t* p);
void tasking_evict_proc_entry(proc_t* p);
void tasking_evict_zombies_waiting_for(proc_t* p);

pid_t tasking_get_proc_count();

/**
 * SYSCALL IMPLEMENTATION
 */

void tasking_fork();
int tasking_exec(const char __user* path, const char __user** argv, const char __user** env);
void tasking_exit(int exit_code);
int tasking_waitpid(int pid, int* status, int options);
int tasking_signal(thread_t* thread, int signo);

/**
 * SIGNALS
 */

void signal_init();

int signal_set_handler(thread_t* thread, int signo, uintptr_t handler);
int signal_set_allow(thread_t* thread, int signo);
int signal_set_private(thread_t* thread, int signo);
int signal_set_pending(thread_t* thread, int signo);
int signal_rem_pending(thread_t* thread, int signo);

int signal_restore_thread_after_handling_signal(thread_t* thread);
int signal_send(thread_t* thread, int signo);
int signal_dispatch_pending(thread_t* thread);

#endif // _KERNEL_TASKING_TASKING_H