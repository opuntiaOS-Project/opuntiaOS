/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

/**
 * For x86 we can use the same stack both for kernel and kthread operations.
 * For arm we need use 2 stacks: one stack for svc mode and one for sys mode, so
 * we create a bigger stack (2 * page_size) to fit 2 stacks required for arm.
 */
#ifdef __i386__
#define KSTACK_ZONE_SIZE VMM_PAGE_SIZE
#define KSTACK_TOP VMM_PAGE_SIZE
#define USTACK_TOP VMM_PAGE_SIZE
#elif __arm__
#define KSTACK_ZONE_SIZE (2 * VMM_PAGE_SIZE)
#define KSTACK_TOP VMM_PAGE_SIZE
#define USTACK_TOP (2 * VMM_PAGE_SIZE)
#endif

extern void trap_return();
extern void _tasking_jumper();

extern int _thread_setup_kstack(thread_t* thread, uint32_t esp);
int kthread_setup(proc_t* p)
{
    p->pid = proc_alloc_pid();
    p->pgid = p->pid;
    p->uid = 0;
    p->gid = 0;
    p->euid = 0;
    p->egid = 0;
    p->suid = 0;
    p->sgid = 0;
    p->is_kthread = true;
    /* allocating kernel stack */
    p->main_thread = proc_alloc_thread();
    p->main_thread->tid = p->pid;
    p->main_thread->process = p;
    p->main_thread->last_cpu = LAST_CPU_NOT_SET;

    p->main_thread->kstack = zoner_new_zone(KSTACK_ZONE_SIZE);
    if (!p->main_thread->kstack.start) {
        return -ENOMEM;
    }
    _thread_setup_kstack(p->main_thread, p->main_thread->kstack.start + KSTACK_TOP);

    /* setting dentries */
    p->proc_file = NULL;
    p->cwd = NULL;

    p->fds = NULL;

    /* setting signal handlers to 0 */
    p->main_thread->signals_mask = 0x0; /* All signals are disabled. */
    p->main_thread->pending_signals_mask = 0x0;
    memset((void*)p->main_thread->signal_handlers, 0, sizeof(p->main_thread->signal_handlers));

    return 0;
}

int kthread_setup_regs(proc_t* p, void* entry_point)
{
    tf_setup_as_kernel_thread(p->main_thread->tf);
    uint32_t stack = (p->main_thread->kstack.start + USTACK_TOP);
    set_base_pointer(p->main_thread->tf, stack);
    set_stack_pointer(p->main_thread->tf, stack);
    set_instruction_pointer(p->main_thread->tf, (uint32_t)entry_point);
    return 0;
}

int kthread_fill_up_stack(thread_t* thread, void* data)
{
    if (!thread) {
        return -EFAULT;
    }
    if (!thread->process->is_kthread) {
        return -EPERM;
    }
    if (!vmm_is_kernel_address((uint32_t)data) && data) {
        return -EFAULT;
    }

#ifdef __i386__
    tf_move_stack_pointer(thread->tf, -sizeof(data));
    vmm_copy_to_pdir(thread->process->pdir, &data, get_stack_pointer(thread->tf), sizeof(data));
#elif __arm__
    thread->tf->r[0] = (uint32_t)data;
#endif
    return 0;
}

int kthread_free(proc_t* p)
{
    /* proc_free can free kthreads too) */
    return proc_free(p);
}