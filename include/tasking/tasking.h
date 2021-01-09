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
#include <tasking/proc.h>
#include <tasking/thread.h>
#include <types.h>
#include <platform/x86/idt.h>

#define CPU_CNT 1
#define THIS_CPU (&cpus[0])
#define RUNNIG_THREAD cpus[0].running_thread
#define MAX_PROCESS_COUNT 1024
#define MAX_DYING_PROCESS_COUNT 8
#define MAX_OPENED_FILES 16
#define SIGNALS_CNT 32

typedef struct {
    char* kstack;
    context_t* scheduler; // context of sched's registers
    thread_t* running_thread;
} __attribute__((packed)) cpu_t;

extern cpu_t cpus[CPU_CNT];
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
int tasking_create_kernel_thread(void* entry_point);

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

#endif // __oneOS__X86__TASKING__TASKING_H