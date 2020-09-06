/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <mem/kmalloc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>
#include <x86/common.h>
#include <x86/gdt.h>
#include <x86/idt.h>
#include <x86/tss.h>

cpu_t cpus[CPU_CNT];
proc_t proc[MAX_PROCESS_COUNT];
uint32_t nxt_proc;

/**
 * CPU FUNCTIONS
 */

/* switching the page dir and tss to the current proc */
void switchuvm(thread_t* thread)
{
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss) - 1, 0);
    tss.esp0 = (uint32_t)(thread->kstack.start + VMM_PAGE_SIZE);
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    RUNNIG_THREAD = thread;
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(thread->process->pdir);
}

/**
 * used to jump to trapend
 * the jump will start the process
 */
void _tasking_jumper()
{
    sti();
    return;
}

/**
 * TASK LOADING FUNCTIONS
 */

extern thread_t thread_storage[512];
extern int threads_cnt;
thread_t* tasking_get_thread(uint32_t tid)
{
    for (int i = 0; i < threads_cnt; i++) {
        if (thread_storage[i].tid == tid) {
            return &thread_storage[i];
        }
    }
    return 0;
}

proc_t* tasking_get_proc(uint32_t pid)
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->pid == pid) {
            return p;
        }
    }
    return 0;
}

proc_t* tasking_get_proc_by_pdir(pdirectory_t* pdir)
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->pdir == pdir) {
            return p;
        }
    }
    return 0;
}

static proc_t* _tasking_alloc_proc()
{
    proc_t* p = &proc[nxt_proc++];
    proc_setup(p);
    return p;
}

static proc_t* _tasking_alloc_kernel_thread(void* entry_point)
{
    proc_t* p = &proc[nxt_proc++];
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
    proc_t* p = _tasking_alloc_proc();
    proc_setup_tty(p, tty_new());

    /* creating new pdir */
    p->pdir = vmm_new_user_pdir();

    if (proc_load(p, "/boot/init") < 0) {
        kprintf("Failed to load init proc");
        while (1) { }
    }

    sched_enqueue(p->main_thread);
}

int tasking_create_kernel_thread(void* entry_point)
{
    proc_t* p = _tasking_alloc_kernel_thread(entry_point);
    p->pdir = vmm_get_kernel_pdir();
    p->main_thread->status = THREAD_RUNNING;
    sched_enqueue(p->main_thread);
    return 0;
}

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init()
{
    nxt_proc = 0;
    signal_init();
}

void tasking_kill_dying()
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->status == PROC_DYING) {
            proc_free(p);
            p->status = PROC_DEAD;
        }
    }
}

/**
 * SYSCALL IMPLEMENTATION
 */

/* Syscall */
void tasking_fork(trapframe_t* tf)
{
    proc_t* new_proc = _tasking_alloc_proc();
    new_proc->pdir = vmm_new_forked_user_pdir();

    /* copying data from proc to new proc */
    proc_copy_of(new_proc, RUNNIG_THREAD);

    /* setting output */
    new_proc->main_thread->tf->eax = 0;
    RUNNIG_THREAD->tf->eax = new_proc->pid;

    /*  After copying the task we need to flush tlb. To do that we need
        to reload cr3 register with a new pdir. To not waste our resources
        we will simply run other process and of course pdir will be refreshed. */
    new_proc->main_thread->status = THREAD_RUNNING;
    sched_enqueue(new_proc->main_thread);
    resched();
}

static int _tasking_do_exec(proc_t* p, const char* path, int argc, char** argv, char** env)
{
    int res = proc_load(p, path);
    if (res == 0) {
        thread_fill_up_stack(p->main_thread, argc, argv, env);
    }
    return res;
}

/* Syscall */
/* TODO: Posix & zeroing-on-demand */
int tasking_exec(const char* path, const char** argv, const char** env)
{
    thread_t* thread = RUNNIG_THREAD;
    proc_t* p = RUNNIG_THREAD->process;
    char* kpath = 0;
    int kargc = 0;
    char** kargv = 0;
    char** kenv = 0;

    if (!str_validate_len(path, 128)) {
        return -EINVAL;
    }
    kpath = kmem_bring_to_kernel(path, strlen(path) + 1);

    if (argv) {
        if (!ptrarr_validate_len(argv, 128)) {
            return -EINVAL;
        }
        kargc = ptrarr_len(argv);

        /* Validating arguments size */
        uint32_t data_len = 0;
        for (int argi = 0; argi < kargc; argi++) {
            if (!str_validate_len(argv[argi], 128)) {
                return -EINVAL;
            }
            data_len += strlen(argv[argi]) + 1;
            if (data_len > 128) {
                return -EINVAL;
            }
        }

        kargv = kmem_bring_to_kernel_ptrarr(argv, kargc);
    }

    /* Cleaning proc */
    dynamic_array_clear(&p->zones);
    proc_kill_all_threads_except(p, thread);
    p->main_thread = thread;
    p->pid = thread->tid;

    int ret = _tasking_do_exec(p, kpath, kargc, kargv, 0);

    kfree(kpath);
    for (int argi = 0; argi < kargc; argi++) {
        kfree(kargv[argi]);
    }
    kfree(kargv);

    return ret;
}

int tasking_waitpid(int pid)
{
    thread_t* thread = RUNNIG_THREAD;
    thread_t* joinee_thread = tasking_get_thread(pid);
    if (!joinee_thread) {
        return -ESRCH;
    }
    thread->joinee = joinee_thread;
    init_join_blocker(thread);
    resched();
    return 0;
}

/* Syscall */
void tasking_exit(int exit_code)
{
    proc_t* proc = RUNNIG_THREAD->process;
    proc->main_thread->exit_code = exit_code;
    proc_die(proc);
    resched();
}