/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

extern void trap_return();
extern void _tasking_jumper();

int _thread_setup_kstack(thread_t* thread, uint32_t esp)
{
    char* sp = (char*)(esp);
    /* setting trapframe in kernel stack */
    sp -= sizeof(*thread->tf);
    thread->tf = (trapframe_t*)sp;

    /* setting return point in kernel stack, so it
       will return to this address in _tasking_jumper */
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)trap_return;

    /* setting context in kernel stack */
    sp -= sizeof(*thread->context);
    thread->context = (context_t*)sp;

    /* setting init data */
    memset((void*)thread->context, 0, sizeof(*thread->context));
    context_set_instruction_pointer(thread->context, (uint32_t)_tasking_jumper);
    memset((void*)thread->tf, 0, sizeof(*thread->tf));

#ifdef FPU_ENABLED
    /* setting fpu */
    thread->fpu_state = kmalloc_aligned(sizeof(fpu_state_t), 16);
    fpu_reset_state(thread->fpu_state);
#endif
    return 0;
}

int thread_setup_main(proc_t* p, thread_t* thread)
{
    /* allocating kernel stack */
    thread->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!thread->kstack.start) {
        return -ENOMEM;
    }

    thread->process = p;
    thread->tid = p->pid;

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
    return 0;
}

int thread_setup(proc_t* p, thread_t* thread)
{
    /* allocating kernel stack */
    thread->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!thread->kstack.start) {
        return -ENOMEM;
    }

    thread->process = p;
    thread->tid = proc_alloc_pid();

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
    return 0;
}

/**
 * STACK FUNCTIONS
 */

int thread_fill_up_stack(thread_t* thread, int argc, char** argv, char** env)
{
    /* TODO: Add env */
    uint32_t argv_data_size = 0;
    for (int i = 0; i < argc; i++) {
        argv_data_size += strlen(argv[i]) + 1;
    }

    if (argv_data_size % 4) {
        argv_data_size += 4 - (argv_data_size % 4);
    }

    uint32_t data_size_on_stack = argv_data_size + (argc + 1) * sizeof(char*) + sizeof(argc) + sizeof(char*);
    int* tmp_buf = (int*)kmalloc(data_size_on_stack);
    if (!tmp_buf) {
        return -EAGAIN;
    }
    memset((void*)tmp_buf, 0, data_size_on_stack);

    char* tmp_buf_ptr = ((char*)tmp_buf) + data_size_on_stack;
    char* tmp_buf_data_ptr = tmp_buf_ptr - argv_data_size;
    uint32_t* tmp_buf_array_ptr = (uint32_t*)((char*)tmp_buf_data_ptr - (argc + 1) * sizeof(char*));
    int* tmp_buf_argv_ptr = (int*)((char*)tmp_buf_array_ptr - sizeof(char*));
    int* tmp_buf_argc_ptr = (int*)((char*)tmp_buf_argv_ptr - sizeof(int));

    uint32_t data_esp = get_stack_pointer(thread->tf) - argv_data_size;
    uint32_t array_esp = data_esp - (argc + 1) * sizeof(char*);
    uint32_t argv_esp = array_esp - 4;
    uint32_t argc_esp = argv_esp - 4;
    uint32_t end_esp = argc_esp; // Points to the end on the stack

    for (int i = argc - 1; i >= 0; i--) {
        uint32_t len = strlen(argv[i]);
        tmp_buf_ptr -= len + 1;
        tf_move_stack_pointer(thread->tf, -(len + 1));
        memcpy(tmp_buf_ptr, argv[i], len);
        tmp_buf_ptr[len] = 0;

        tmp_buf_array_ptr[i] = get_stack_pointer(thread->tf);
    }
    tmp_buf_array_ptr[argc] = 0;

    // FIXME: Remove these elements from stack for ARM
    *tmp_buf_argv_ptr = array_esp;
    *tmp_buf_argc_ptr = argc;

    set_stack_pointer(thread->tf, end_esp);
#ifdef __arm__
    thread->tf->r[0] = argc;
    thread->tf->r[1] = array_esp;
#endif

    vmm_copy_to_pdir(thread->process->pdir, (uint8_t*)tmp_buf, get_stack_pointer(thread->tf), data_size_on_stack);

    kfree(tmp_buf);

    return 0;
}

int thread_free(thread_t* thread)
{
    thread->status = THREAD_DEAD;
    // Don't dequeue here, since thread_die has been already run.
    zoner_free_zone(thread->kstack);
    return 0;
}

int thread_die(thread_t* thread)
{
    thread->status = THREAD_DYING;
    sched_dequeue(thread);
    return 0;
}

/**
 * DEBUG FUNCTIONS
 */

int thread_dump_frame(thread_t* thread)
{
#ifdef __i386__
    for (uint32_t i = thread->tf->esp; i < thread->tf->ebp; i++) {
        uint8_t byte = *(uint8_t*)i;
        uint32_t b32 = (uint32_t)byte;
        log("%x - %x\n", i, b32);
    }
#endif
    return 0;
}