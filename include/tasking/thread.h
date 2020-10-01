/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__TASKING__THREAD_H
#define __oneOS__X86__TASKING__THREAD_H

#include <fs/vfs.h>
#include <tasking/signal.h>
#include <types.h>
#include <x86/idt.h>
#include <time/time_manager.h>

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} __attribute__((packed)) context_t;

enum THREAD_STATUS {
    THREAD_INVALID = 0,
    THREAD_RUNNING,
    THREAD_DEAD,
    THREAD_STOPPED,
    THREAD_BLOCKED,
    THREAD_DYING,
};

struct thread;
struct blocker {
    int reason;
    int (*should_unblock)(struct thread* p);
    bool should_unblock_for_signal;
};
typedef struct blocker blocker_t;

enum BLOCKER_REASON {
    BLOCKER_INVALID,
    BLOCKER_JOIN,
    BLOCKER_READ,
    BLOCKER_WRITE,
    BLOCKER_SLEEP,
    BLOCKER_SELECT,
};

struct proc;
struct thread {
    struct proc* process;
    uint32_t tid;
    uint32_t status;

    /* Kernel data */
    zone_t kstack;
    context_t* context; // context of kernel's registers
    trapframe_t* tf;

    /* Scheduler data */
    struct thread* sched_prev;
    struct thread* sched_next;

    /* Blocker data */
    blocker_t blocker;
    int exit_code;
    struct thread* joinee;
    file_descriptor_t* blocker_fd;
    time_t unblock_time;
    int nfds;
    fd_set_t readfds;
    fd_set_t writefds;
    fd_set_t exceptfds;

    uint32_t signals_mask;
    uint32_t pending_signals_mask;
    void* signal_handlers[SIGNALS_CNT];
};
typedef struct thread thread_t;

/**
 * THREAD FUNCTIONS
 */

int thread_setup_main(struct proc* p, thread_t* thread);
int thread_setup(struct proc* p, thread_t* thread);
int thread_setup_kstack(thread_t* thread);

void thread_set_stack(thread_t* thread, uint32_t ebp, uint32_t esp);
void thread_set_eip(thread_t* thread, uint32_t eip);

int thread_fill_up_stack(thread_t* thread, int argc, char** argv, char** env);

int thread_free(thread_t* thread);
int thread_die(thread_t* thread);

/**
 * BLOCKER FUNCTIONS
 */

int init_join_blocker(thread_t* p);
int init_read_blocker(thread_t* p, file_descriptor_t* bfd);
int init_write_blocker(thread_t* thread, file_descriptor_t* bfd);
int init_sleep_blocker(thread_t* thread, uint32_t time);
int init_select_blocker(thread_t* thread, int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout);

/**
 * DEBUG FUNCTIONS
 */

int thread_dump_frame(thread_t* thread);

#endif /* __oneOS__X86__TASKING__THREAD_H */