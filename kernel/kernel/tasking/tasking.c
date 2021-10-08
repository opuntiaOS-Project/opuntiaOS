/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef FPU_ENABLED
#include <drivers/x86/fpu.h>
#endif
#include <io/tty/tty.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/system.h>
#include <tasking/cpu.h>
#include <tasking/dump.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

#define TASKING_DEBUG

cpu_t cpus[CPU_CNT];
proc_t proc[MAX_PROCESS_COUNT];
static uint32_t nxt_proc = 0;

static int _tasking_do_exec(proc_t* p, thread_t* main_thread, const char* path, int argc, char** argv, int envc, char** envp);

static inline uint32_t _tasking_next_proc_id()
{
    return atomic_add(&nxt_proc, 1) - 1;
}

static inline uint32_t _tasking_get_proc_count()
{
    return atomic_load(&nxt_proc);
}

/**
 * used to jump to trapend
 * the jump will start the process
 */
#ifdef __i386__
void _tasking_jumper()
{
    cpu_leave_kernel_space();
    system_enable_interrupts_only_counter();
    return;
}
#endif

/**
 * TASK LOADING FUNCTIONS
 */

extern thread_list_t thread_list;
thread_t* tasking_get_thread(uint32_t tid)
{
    thread_list_node_t* __thread_list_node = thread_list.head;
    while (__thread_list_node) {
        for (int i = 0; i < THREADS_PER_NODE; i++) {
            if (__thread_list_node->thread_storage[i].tid == tid) {
                return &__thread_list_node->thread_storage[i];
            }
        }
        __thread_list_node = __thread_list_node->next;
    }

    return NULL;
}

proc_t* tasking_get_proc(uint32_t pid)
{
    proc_t* p;
    for (int i = 0; i < _tasking_get_proc_count(); i++) {
        p = &proc[i];
        if (p->pid == pid) {
            return p;
        }
    }
    return NULL;
}

proc_t* tasking_get_proc_by_pdir(pdirectory_t* pdir)
{
    proc_t* p;
    for (int i = 0; i < _tasking_get_proc_count(); i++) {
        p = &proc[i];
        if (p->status == PROC_ALIVE && p->pdir == pdir) {
            return p;
        }
    }
    return NULL;
}

static inline proc_t* _tasking_alloc_proc()
{
    proc_t* p = &proc[_tasking_next_proc_id()];
    lock_init(&p->lock);
    return p;
}

static proc_t* _tasking_setup_proc()
{
    proc_t* p = _tasking_alloc_proc();
    proc_setup(p);
    return p;
}

static proc_t* _tasking_setup_proc_with_uid(uid_t uid, gid_t gid)
{
    proc_t* p = _tasking_alloc_proc();
    proc_setup_with_uid(p, uid, gid);
    return p;
}

static proc_t* _tasking_fork_proc_from_current()
{
    proc_t* new_proc = _tasking_setup_proc();
    new_proc->pdir = vmm_new_forked_user_pdir();
    proc_copy_of(new_proc, RUNNING_THREAD);
    return new_proc;
}

static proc_t* _tasking_alloc_kernel_thread(void* entry_point)
{
    proc_t* p = _tasking_alloc_proc();
    kthread_setup(p);
    kthread_setup_regs(p, entry_point);
    return p;
}

/**
 * Start init proccess
 * All others processes will fork
 */
void tasking_start_init_proc()
{
    // We need to stop interrupts here, since this part of code
    // is NOT interruptable.
    system_disable_interrupts();
    proc_t* p = _tasking_setup_proc_with_uid(0, 0);
    proc_setup_tty(p, tty_new());

    int err = _tasking_do_exec(p, p->main_thread, "/boot/init", 0, NULL, 0, NULL);
    if (err) {
        kpanic("Failed to load init proc");
    }

    sched_enqueue(p->main_thread);
    system_enable_interrupts();
}

proc_t* tasking_create_kernel_thread(void* entry_point, void* data)
{
    proc_t* p = _tasking_alloc_kernel_thread(entry_point);
    p->pdir = vmm_get_kernel_pdir();
    kthread_fill_up_stack(p->main_thread, data);
    p->main_thread->status = THREAD_STATUS_RUNNING;
    return p;
}

proc_t* tasking_run_kernel_thread(void* entry_point, void* data)
{
    proc_t* p = tasking_create_kernel_thread(entry_point, data);
    sched_enqueue(p->main_thread);
    return p;
}

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init()
{
    proc_init_storage();
    signal_init();
    dump_prepare_kernel_data();
}

void tasking_kill_dying()
{
    proc_t* p;
    for (int i = 0; i < _tasking_get_proc_count(); i++) {
        p = &proc[i];
        if (p->status == PROC_DYING) {
            lock_acquire(&p->lock);
            if (unlikely(p->status != PROC_DYING)) {
                lock_release(&p->lock);
                continue;
            }
            proc_free_lockless(p);
            p->status = PROC_DEAD;
            lock_release(&p->lock);
        }
    }
}

/**
 * SYSCALL IMPLEMENTATION
 */

void tasking_fork(trapframe_t* tf)
{
    proc_t* new_proc = _tasking_fork_proc_from_current();

    /* setting output */
    set_syscall_result(new_proc->main_thread->tf, 0);
    set_syscall_result(RUNNING_THREAD->tf, new_proc->pid);

    new_proc->main_thread->status = THREAD_STATUS_RUNNING;

#ifdef TASKING_DEBUG
    log("Fork %d to pid %d", RUNNING_THREAD->tid, new_proc->pid);
#endif

    sched_enqueue(new_proc->main_thread);
    resched();
}

static int _tasking_validate_exec_params(const char** argv, int* kargc, char*** kargv)
{
    int start_with = *kargc;
    if (argv) {
        if (!ptrarr_validate_len(argv, 128)) {
            return -EINVAL;
        }

        int argc = ptrarr_len(argv);
        *kargc += argc;

        /* Validating arguments size */
        uint32_t data_len = 0;
        for (int argi = 0; argi < argc; argi++) {
            if (!str_validate_len(argv[argi], 128)) {
                return -EINVAL;
            }
            data_len += strlen(argv[argi]) + 1;
            if (data_len > 128) {
                return -EINVAL;
            }
        }
    }

    char** res = NULL;
    res = kmalloc(*kargc * sizeof(char*));

    // Inlined part of kmem_bring_to_kernel_ptrarr
    for (int i = start_with; i < *kargc; i++) {
        res[i] = kmem_bring_to_kernel(argv[i - 1], strlen(argv[i - 1]) + 1);
    }

    *kargv = res;
    return 0;
}

static int _tasking_do_exec(proc_t* p, thread_t* main_thread, const char* path, int argc, char** argv, int envc, char** envp)
{
    int err = proc_load(p, main_thread, path);
    if (err) {
        return err;
    }
    return thread_fill_up_stack(p->main_thread, argc, argv, envc, envp);
}

int tasking_exec(const char* path, const char** argv, const char** envp)
{
    thread_t* thread = RUNNING_THREAD;
    proc_t* p = RUNNING_THREAD->process;
    char* kpath = NULL;
    int kargc = 1;
    char** kargv = NULL;
    int kenvc = 0;
    char** kenv = NULL;

    if (!str_validate_len(path, 128)) {
        return -EINVAL;
    }
    kpath = kmem_bring_to_kernel(path, strlen(path) + 1);

    int err = _tasking_validate_exec_params(argv, &kargc, &kargv);
    if (err) {
        goto exit;
    }
    kargv[0] = kpath;

    err = _tasking_validate_exec_params(envp, &kenvc, &kenv);
    if (err) {
        goto exit;
    }

    err = _tasking_do_exec(p, thread, kpath, kargc, kargv, kenvc, kenv);

#ifdef TASKING_DEBUG
    if (!err) {
        log("Exec %s : pid %d", kpath, p->pid);
    }
#endif

exit:
    if (kpath) {
        kfree(kpath);
    }
    if (kargv) {
        for (int argi = 0; argi < kargc; argi++) {
            kfree(kargv[argi]);
        }
        kfree(kargv);
    }
    if (kenv) {
        for (int argi = 0; argi < kenvc; argi++) {
            kfree(kenv[argi]);
        }
        kfree(kenv);
    }

    return err;
}

int tasking_waitpid(int pid, int* status)
{
    thread_t* thread = RUNNING_THREAD;
    thread_t* joinee_thread = tasking_get_thread(pid);
    if (!joinee_thread) {
        return -ESRCH;
    }
    thread->joinee = joinee_thread;
    init_join_blocker(thread);

    // FIXME: Status just return exit code.
    int kstatus = thread->joinee->exit_code;
    if (status) {
        vmm_copy_to_user(status, &kstatus, sizeof(int));
    }
    return 0;
}

void tasking_exit(int exit_code)
{
    proc_t* p = RUNNING_THREAD->process;
    p->main_thread->exit_code = exit_code;
    proc_die(p);
    resched();
}

int tasking_kill(thread_t* thread, int signo)
{
    if (thread->status == THREAD_STATUS_INVALID || thread->status == THREAD_STATUS_DEAD || thread->status == THREAD_STATUS_DYING) {
        return -EINVAL;
    }
    signal_set_pending(thread, signo);

    // If the target thread is a one that issued kill to self,
    // dispatch the signal right now. Overwise scheduler will
    // dispatch when it switches to the task.
    if (RUNNING_THREAD == thread) {
        signal_dispatch_pending(thread);
    }
    return 0;
}