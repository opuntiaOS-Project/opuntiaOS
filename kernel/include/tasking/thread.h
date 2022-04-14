/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_THREAD_H
#define _KERNEL_TASKING_THREAD_H

#include <drivers/generic/fpu.h>
#include <fs/vfs.h>
#include <libkern/lock.h>
#include <libkern/types.h>
#include <platform/generic/tasking/context.h>
#include <platform/generic/tasking/trapframe.h>
#include <tasking/signal.h>
#include <time/time_manager.h>

enum THREAD_STATUS {
    THREAD_STATUS_INVALID = 0,
    THREAD_STATUS_ALLOCATED,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_STOPPED,
    THREAD_STATUS_BLOCKED,
    THREAD_STATUS_DYING,
};

struct thread;
struct blocker {
    int reason;
    bool (*should_unblock)(struct thread* thread);
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
    BLOCKER_DUMPING,
    BLOCKER_STOP, // Just waiting for signal which will continue the thread.
};

struct blocker_join {
    struct thread* joinee;
    int join_pid;
};
typedef struct blocker_join blocker_join_t;

struct blocker_rw {
    file_descriptor_t* fd;
};
typedef struct blocker_rw blocker_rw_t;

struct blocker_sleep {
    time_t until;
};
typedef struct blocker_sleep blocker_sleep_t;

struct blocker_select {
    int nfds;
    fd_set_t readfds;
    fd_set_t writefds;
    fd_set_t exceptfds;
};
typedef struct blocker_select blocker_select_t;

struct proc;
struct thread {
    struct proc* process;
    pid_t tid;
    uint32_t status;

    /* Kernel data */
    kmemzone_t kstack;
    context_t* context; // context of kernel's registers
    trapframe_t* tf;
    fpu_state_t* fpu_state;

    /* Scheduler data */
    struct thread* sched_prev;
    struct thread* sched_next;
    int last_cpu;
    time_t ticks_until_preemption;
    time_t start_time_in_ticks; // Time when the task was put to run.

    /* Blocker data */
    blocker_t blocker;
    union {
        blocker_join_t join;
        blocker_rw_t rw;
        blocker_sleep_t sleep;
        blocker_select_t select;
    } blocker_data;

    /* Stat data */
    time_t stat_total_running_ticks;

    uint32_t signals_mask;
    uint32_t pending_signals_mask;
    void* signal_handlers[SIGNALS_CNT];
};
typedef struct thread thread_t;

#define THREADS_PER_NODE (128)
struct thread_list_node {
    thread_t thread_storage[THREADS_PER_NODE];
    struct thread_list_node* next;
    int empty_spots;
};
typedef struct thread_list_node thread_list_node_t;

struct thread_list {
    struct thread_list_node* head;
    struct thread_list_node* next_empty_node;
    int next_empty_index;
    spinlock_t lock;
    struct thread_list_node* tail;
};
typedef struct thread_list thread_list_t;

/**
 * THREAD FUNCTIONS
 */

int thread_setup_main(struct proc* p, thread_t* thread);
int thread_setup(struct proc* p, thread_t* thread);
int thread_setup_kstack(thread_t* thread);
int thread_copy_of(thread_t* thread, thread_t* from_thread);

int thread_fill_up_stack(thread_t* thread, int argc, char** argv, int envc, char** envp);

int thread_kstack_free(thread_t* thread);
int thread_free(thread_t* thread);
int thread_die(thread_t* thread);
int thread_zombie(thread_t* thread);
int thread_stop(thread_t* thread);
int thread_stop_and_resched(thread_t* thread);
int thread_continue(thread_t* thread);

static ALWAYS_INLINE int thread_is_freed(thread_t* thread) { return (thread->status == THREAD_STATUS_INVALID); }
static ALWAYS_INLINE int thread_is_alive(thread_t* thread) { return !thread_is_freed(thread); }

/**
 * BLOCKER FUNCTIONS
 */

int thread_init_blocker(thread_t* thread, const struct blocker* blocker);

int init_join_blocker(thread_t* thread, int wait_for_pid);
int init_read_blocker(thread_t* thread, file_descriptor_t* bfd);
int init_write_blocker(thread_t* thread, file_descriptor_t* bfd);
int init_sleep_blocker(thread_t* thread, time_t time);
int init_select_blocker(thread_t* thread, int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout);

/**
 * DEBUG FUNCTIONS
 */

int thread_dump_frame(thread_t* thread);
int thread_print_backtrace();

#endif /* _KERNEL_TASKING_THREAD_H */