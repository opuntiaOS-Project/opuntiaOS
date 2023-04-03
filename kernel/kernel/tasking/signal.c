/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/bits/signal.h>
#include <libkern/libkern.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/signal_impl.h>
#include <tasking/dump.h>
#include <tasking/sched.h>
#include <tasking/signal.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

#define MAGIC_STATE_JUST_TF 0xfeed3eee
#define MAGIC_STATE_NEW_STACK 0xea12002a

#ifdef __i386__
#define return_tf (thread->tf->ebx)
#elif __x86_64__
#define return_tf (thread->tf->rdi)
#elif __arm__
#define return_tf (thread->tf->r[1])
#elif __aarch64__
#define return_tf (thread->tf->x[1])
#elif defined(__riscv) && (__riscv_xlen == 64)
#define return_tf (thread->tf->a0)
#endif

static kmemzone_t _signal_jumper_zone;

extern void signal_caller_start();
extern void signal_caller_end();

/**
 * INIT
 */

static void _signal_init_caller()
{
    _signal_jumper_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_alloc_page(_signal_jumper_zone.start, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC | MMU_FLAG_PERM_READ | MMU_FLAG_NONPRIV);
    size_t signal_caller_len = (size_t)signal_caller_end - (size_t)signal_caller_start;
    memcpy(_signal_jumper_zone.ptr, (void*)signal_caller_start, signal_caller_len);
    vmm_tune_page(_signal_jumper_zone.start, MMU_FLAG_PERM_EXEC | MMU_FLAG_PERM_READ | MMU_FLAG_NONPRIV);
}

void signal_init()
{
    _signal_init_caller();
}

/**
 * HELPER FUNCTIONS
 */

int signal_set_handler(thread_t* thread, int signo, uintptr_t handler)
{
    if (signo < 0 || signo >= SIGNALS_CNT || signo == SIGSTOP || signo == SIGKILL) {
        return -EINVAL;
    }

    thread->signal_handlers[signo] = (void*)handler;
    return 0;
}

/**
 * Makes signal allow to be called
 */
int signal_set_allow(thread_t* thread, int signo)
{
    if (signo < 0 || signo >= SIGNALS_CNT) {
        return -EINVAL;
    }
    thread->signals_mask |= (1 << signo);
    return 0;
}

/**
 * Makes signal private. It can't be called.
 */
int signal_set_private(thread_t* thread, int signo)
{
    if (signo < 0 || signo >= SIGNALS_CNT) {
        return -EINVAL;
    }
    thread->signals_mask &= ~((uint32_t)(1 << signo));
    return 0;
}

int signal_set_pending(thread_t* thread, int signo)
{
    if (signo < 0 || signo >= SIGNALS_CNT) {
        return -EINVAL;
    }
    thread->pending_signals_mask |= (1 << signo);
    return 0;
}

int signal_rem_pending(thread_t* thread, int signo)
{
    if (signo < 0 || signo >= SIGNALS_CNT) {
        return -EINVAL;
    }
    thread->pending_signals_mask &= ~((uint32_t)(1 << signo));
    return 0;
}

extern int _thread_setup_kstack(thread_t* thread, uintptr_t sp);
static int signal_setup_stack_to_handle_signal(thread_t* thread, int signo)
{
    system_disable_interrupts();
    vm_address_space_t* prev_aspace = vmm_get_active_address_space();
    vmm_switch_address_space(thread->process->address_space);
    vmm_ensure_writing_to_active_address_space((uintptr_t)thread->tf, sizeof(*thread->tf));

    uintptr_t old_sp = get_stack_pointer(thread->tf);
    uintptr_t magic = MAGIC_STATE_JUST_TF; /* helps to restore thread after signal to the right state */

    if (thread != RUNNING_THREAD) {
        // If we are here that means that the thread was stopped while
        // being in kernel (because of scheduler or blocker). That means,
        // we need not to corrupt it's kernel state, so we create a new state
        // to send signal.

        // We setup a new kernel state upper the previous one
        // Stack:
        //     Trapframe
        //     Function calls and local vars...
        //     Context
        // ---- added after setup ----
        //     Trapframe for signal
        //     Context for signal

        magic = MAGIC_STATE_NEW_STACK;
        trapframe_t* old_tf = thread->tf;
        context_t* old_ctx = thread->context;

        const size_t alignment = sizeof(uintptr_t) * 2;
        uintptr_t new_tf_loc = ROUND_FLOOR((uintptr_t)thread->context, alignment);

        _thread_setup_kstack(thread, new_tf_loc);
        memcpy(thread->tf, old_tf, sizeof(trapframe_t));

        tf_push_to_stack(thread->tf, (uintptr_t)old_tf);
        tf_push_to_stack(thread->tf, (uintptr_t)old_ctx);
        tf_push_to_stack(thread->tf, (uintptr_t)old_tf ^ (uintptr_t)old_ctx); // checksum
    }

    signal_impl_prepare_stack(thread, signo, old_sp, magic);
    vmm_switch_address_space(prev_aspace);
    system_enable_interrupts();
    return 0;
}

int signal_restore_thread_after_handling_signal(thread_t* thread)
{
    int ret = return_tf;

    uintptr_t old_sp, magic;
    signal_impl_restore_stack(thread, &old_sp, &magic);

    if (magic == MAGIC_STATE_NEW_STACK) {
        uintptr_t checksum = tf_pop_to_stack(thread->tf);
        context_t* old_ctx = (context_t*)tf_pop_to_stack(thread->tf);
        trapframe_t* old_tf = (trapframe_t*)tf_pop_to_stack(thread->tf);

        uintptr_t calced_checksum = ((uintptr_t)old_tf ^ (uintptr_t)old_ctx);

        if (checksum != calced_checksum) {
            log_error("Killed %d: wrong signal checksum\n", thread->process->pid);
            proc_die(thread->process, 9);
            resched_dont_save_context();
        }

        thread->tf = old_tf;
        thread->context = old_ctx;
    }

    if (old_sp != get_stack_pointer(thread->tf)) {
        log_error("SPs are diff after signal %zx != %zx", old_sp, get_stack_pointer(thread->tf));
        proc_die(thread->process, 9);
        resched_dont_save_context();
    }

    // If our thread is blocked, that means that it already has a context on stack, no need to overwrite it.
    if (thread->blocker.reason != BLOCKER_INVALID) {
        thread->status = THREAD_STATUS_BLOCKED;
        sched_dequeue(thread);
        resched_dont_save_context();
    }

    // Since we already have a context on the stack, no need to overwrite it.
    if (magic == MAGIC_STATE_NEW_STACK) {
        resched_dont_save_context();
    }

    return ret;
}

static int signal_default_action(int signo)
{
    if (signo < 0 || signo >= SIGNALS_CNT) {
        return SIGNAL_ACTION_ABNORMAL_TERMINATE;
    }

    static const int defact[] = {
        [SIGABRT] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGALRM] = SIGNAL_ACTION_TERMINATE,
        [SIGBUS] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGCHLD] = SIGNAL_ACTION_IGNORE,
        [SIGCONT] = SIGNAL_ACTION_CONTINUE,
        [SIGFPE] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGHUP] = SIGNAL_ACTION_TERMINATE,
        [SIGILL] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGINT] = SIGNAL_ACTION_TERMINATE,
        [SIGKILL] = SIGNAL_ACTION_TERMINATE,
        [SIGPIPE] = SIGNAL_ACTION_TERMINATE,
        [SIGQUIT] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGSEGV] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGSTOP] = SIGNAL_ACTION_STOP,
        [SIGTERM] = SIGNAL_ACTION_TERMINATE,
        [SIGTSTP] = SIGNAL_ACTION_STOP,
        [SIGTTIN] = SIGNAL_ACTION_STOP,
        [SIGTTOU] = SIGNAL_ACTION_STOP,
        [SIGUSR1] = SIGNAL_ACTION_TERMINATE,
        [SIGUSR2] = SIGNAL_ACTION_TERMINATE,
        [SIGPOLL] = SIGNAL_ACTION_TERMINATE,
        [SIGPROF] = SIGNAL_ACTION_TERMINATE,
        [SIGSYS] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGTRAP] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGURG] = SIGNAL_ACTION_IGNORE,
        [SIGVTALRM] = SIGNAL_ACTION_TERMINATE,
        [SIGXCPU] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
        [SIGXFSZ] = SIGNAL_ACTION_ABNORMAL_TERMINATE,
    };
    return defact[signo];
}

static int signal_process(thread_t* thread, int signo)
{
    if (thread->signal_handlers[signo] == SIG_IGN) {
        return 0;
    }

    if (thread->signal_handlers[signo]) {
        signal_setup_stack_to_handle_signal(thread, signo);
        set_instruction_pointer(thread->tf, _signal_jumper_zone.start);
        return 0;
    } else {
        int result = signal_default_action(signo);
        switch (result) {
        case SIGNAL_ACTION_TERMINATE:
            proc_die(thread->process, 0);
            return 0;
        case SIGNAL_ACTION_ABNORMAL_TERMINATE:
            dump_and_kill(RUNNING_THREAD->process);
            return 0;
        case SIGNAL_ACTION_IGNORE:
        case SIGNAL_ACTION_STOP:
        case SIGNAL_ACTION_CONTINUE:
            // These actions were processed earlier, right when signals were issued. (look at signal_send_signal)
            return 0;

        default:
            break;
        }
    }
    return -EFAULT;
}

int signal_send(thread_t* thread, int signo)
{
    if (signo == 0) {
        return 0;
    }

    int err = signal_set_pending(thread, signo);
    if (err) {
        return err;
    }

    // Issue STOP and CONT actions, while others will be issued later when
    // the process does not run.
    //
    // TODO: Think of POSIX compliant of recieving signals. Currently we look
    // at default actions only.
    int ret = signal_default_action(signo);
    switch (ret) {
    case SIGNAL_ACTION_CONTINUE:
        if (thread && thread->status == THREAD_STATUS_BLOCKED && thread->blocker.should_unblock_for_signal) {
            sched_enqueue(thread);
        }
        if (thread && thread->status == THREAD_STATUS_STOPPED) {
            sched_enqueue(thread);
        }
        break;

    case SIGNAL_ACTION_STOP:
        // If the thread is a running one, It is safe to
        // stop it in the current state and resched.
        if (thread && thread == RUNNING_THREAD) {
            thread_stop_and_resched(thread);
        } else {
            thread_stop(thread);
        }
        break;

    default:
        break;
    }

    return 0;
}

int signal_dispatch_pending(thread_t* thread)
{
    uint32_t candidate_signals = thread->pending_signals_mask & thread->signals_mask;

    if (!candidate_signals) {
        return -EACCES;
    }

    uint32_t signo = 1;
    for (; signo < SIGNALS_CNT; signo++) {
        if (candidate_signals & (1 << signo)) {
            break;
        }
    }

    signal_rem_pending(thread, signo);
    int err = signal_process(thread, signo);

    if (err) {
        return err;
    }
    return 0;
}
