/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef FPU_ENABLED
#include <drivers/x86/fpu.h>
#endif
#include <io/tty/vconsole.h>
#include <libkern/bits/errno.h>
#include <libkern/bits/sys/wait.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <mem/swapfile.h>
#include <platform/generic/system.h>
#include <tasking/cpu.h>
#include <tasking/dump.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

#define TASKING_DEBUG

proc_t proc[MAX_PROCESS_COUNT];
static pid_t nxt_proc = 0;

static int _tasking_do_exec(proc_t* p, thread_t* main_thread, const char* path, int argc, char** argv, int envc, char** envp);

static inline pid_t _tasking_next_proc_id()
{
    return atomic_add(&nxt_proc, 1) - 1;
}

static inline pid_t _tasking_get_proc_count()
{
    return atomic_load(&nxt_proc);
}

pid_t tasking_get_proc_count()
{
    return _tasking_get_proc_count();
}

/**
 * used to jump to trapend
 * the jump will start the process
 */
#ifdef __i386__
void _tasking_jumper()
{
    cpu_enter_user_space();
#ifndef PREEMPT_KERNEL
    // The jumper is called during the creation of a new thread, thus
    // interrupt counter should not be affected when preemtion is on.
    system_enable_interrupts_only_counter();
#endif
    return;
}
#endif

/**
 * TASK LOADING FUNCTIONS
 */

extern thread_list_t thread_list;
thread_t* tasking_get_thread(pid_t tid)
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

proc_t* tasking_get_proc(pid_t pid)
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

static inline proc_t* _tasking_alloc_proc()
{
    proc_t* p = &proc[_tasking_next_proc_id()];
    spinlock_init(&p->vm_lock);
    spinlock_init(&p->lock);
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
    proc_fork_from(new_proc, RUNNING_THREAD);
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
    system_disable_interrupts();
    proc_t* p = _tasking_setup_proc_with_uid(0, 0);
    proc_setup_vconsole(p, vconsole_new());

    int err = _tasking_do_exec(p, p->main_thread, "/System/launch_server", 0, NULL, 0, NULL);
    if (err) {
        kpanic("Failed to load init proc");
    }

    sched_enqueue(p->main_thread);
    system_enable_interrupts();
}

proc_t* tasking_create_kernel_thread(void* entry_point, void* data)
{
    proc_t* p = _tasking_alloc_kernel_thread(entry_point);
    p->address_space = vmm_get_kernel_address_space();
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
    // swapfile_init();
    // signal_init();
    // dump_prepare_kernel_data();
}

bool tasking_should_become_zombie(proc_t* p)
{
    proc_t* pproc = tasking_get_proc(p->ppid);
    if (!pproc) {
        return false;
    }

    if (!proc_is_alive(pproc)) {
        return false;
    }

    if (pproc->main_thread->signal_handlers[SIGCHLD]) {
        // signal_send(pproc->main_thread, SIGCHLD);
        return false;
    }
    return true;
}

void tasking_evict_zombies_waiting_for(proc_t* pp)
{
    // TODO: Do it more efficient, keep a list of zombies or a hashtable.
    proc_t* p;
    for (int i = 0; i < _tasking_get_proc_count(); i++) {
        p = &proc[i];
        if (p->status == PROC_ZOMBIE && p->ppid == pp->pid) {
            tasking_evict_proc_entry(p);
        }
    }
}

void tasking_evict_proc_entry(proc_t* p)
{
    p->pid = 0;
    p->status = PROC_DEAD;
}

void tasking_kill_dying()
{
    proc_t* p;
    for (int i = 0; i < _tasking_get_proc_count(); i++) {
        p = &proc[i];
        if (p->status == PROC_DYING) {
            spinlock_acquire(&p->vm_lock);
            spinlock_acquire(&p->lock);
            if (unlikely(p->status != PROC_DYING)) {
                spinlock_release(&p->lock);
                spinlock_release(&p->vm_lock);
                continue;
            }
            proc_free_locked(p);
            if (tasking_should_become_zombie(p)) {
                p->status = PROC_ZOMBIE;
            } else {
                tasking_evict_proc_entry(p);
            }
            spinlock_release(&p->lock);
            spinlock_release(&p->vm_lock);
        }
    }
}

/**
 * SYSCALL IMPLEMENTATION
 */

void tasking_fork()
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
        if (!umem_validate_strarray(argv, USER_PTRARRAY_MAXLEN)) {
            return -EINVAL;
        }

        int argc = ptrarray_len((const void**)argv);
        *kargc += argc;

        // Validating arguments size
        size_t data_len = 0;
        for (int argi = 0; argi < argc; argi++) {
            if (!umem_validate_str(argv[argi], USER_STR_MAXLEN)) {
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

    for (int i = start_with; i < *kargc; i++) {
        res[i] = umem_bring_to_kernel(argv[i - start_with], strlen(argv[i - start_with]) + 1);
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

int tasking_exec(const char __user* path, const char __user** argv, const char __user** envp)
{
    thread_t* thread = RUNNING_THREAD;
    proc_t* p = RUNNING_THREAD->process;
    char* kpath = NULL;
    int kargc = 0;
    char** kargv = NULL;
    int kenvc = 0;
    char** kenv = NULL;

    kpath = umem_bring_to_kernel_str(path, USER_STR_MAXLEN);
    if (!kpath) {
        return -EINVAL;
    }

    int err = _tasking_validate_exec_params(argv, &kargc, &kargv);
    if (err) {
        goto exit;
    }

    err = _tasking_validate_exec_params(envp, &kenvc, &kenv);
    if (err) {
        goto exit;
    }

    err = _tasking_do_exec(p, thread, kpath, kargc, kargv, kenvc, kenv);
    if (err) {
        goto exit;
    }

#ifdef TASKING_DEBUG
    log("Exec %s : pid %d", kpath, p->pid);
#endif

    if (p->is_tracee) {
        // Wait for SIGCONT from parent thread.
        tasking_signal(thread, SIGSTOP);
    }

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

int tasking_waitpid(int pid, int* status, int options)
{
    thread_t* thread = RUNNING_THREAD;
    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return -ESRCH;
    }

    if (!TEST_FLAG(options, WNOHANG)) {
        // init_join_blocker(thread, pid);
    }

    if (status) {
        proc_t* p = tasking_get_proc(pid);
        *status = p->exit_code;
    }
    return 0;
}

void tasking_exit(int exit_code)
{
    proc_t* p = RUNNING_THREAD->process;
    p->exit_code = exit_code;
    proc_die(p);
    resched();
}

int tasking_signal(thread_t* thread, int signo)
{
    if (thread->status == THREAD_STATUS_INVALID || thread->status == THREAD_STATUS_DYING) {
        return -EINVAL;
    }
    // signal_send(thread, signo);

    // If the target thread is a one that issued kill to self,
    // dispatch the signal right now. Overwise scheduler will
    // dispatch when it switches to the task.
    if (RUNNING_THREAD == thread) {
        // signal_dispatch_pending(thread);
    }
    return 0;
}