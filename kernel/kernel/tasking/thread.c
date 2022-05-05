/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

int _thread_setup_kstack(thread_t* thread, uintptr_t esp)
{
    char* sp = (char*)(esp);
    /* setting trapframe in kernel stack */
    sp -= sizeof(*thread->tf);
    thread->tf = (trapframe_t*)sp;

    /* setting return point in kernel stack, so it
       will return to this address in _tasking_jumper */
    sp -= sizeof(uintptr_t);
    *(uintptr_t*)sp = (uintptr_t)trap_return;

    /* setting context in kernel stack */
    sp -= sizeof(*thread->context);
    thread->context = (context_t*)sp;

    /* setting init data */
    memset((void*)thread->context, 0, sizeof(*thread->context));
    context_set_instruction_pointer(thread->context, (uintptr_t)_tasking_jumper);
    memset((void*)thread->tf, 0, sizeof(*thread->tf));
    return 0;
}

int thread_setup_main(proc_t* p, thread_t* thread)
{
    /* allocating kernel stack */
    thread->kstack = kmemzone_new(VMM_PAGE_SIZE);
    if (!thread->kstack.start) {
        return -ENOMEM;
    }

    thread->process = p;
    thread->tid = p->pid;
    thread->last_cpu = LAST_CPU_NOT_SET;

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
#ifdef FPU_ENABLED
    /* setting fpu */
    thread->fpu_state = kmalloc_aligned(sizeof(fpu_state_t), FPU_STATE_ALIGNMENT);
    fpu_init_state(thread->fpu_state);
#endif
    return 0;
}

int thread_setup(proc_t* p, thread_t* thread)
{
    /* allocating kernel stack */
    thread->kstack = kmemzone_new(VMM_PAGE_SIZE);
    if (!thread->kstack.start) {
        return -ENOMEM;
    }

    thread->process = p;
    thread->tid = proc_alloc_pid();
    thread->last_cpu = LAST_CPU_NOT_SET;

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
#ifdef FPU_ENABLED
    /* setting fpu */
    thread->fpu_state = kmalloc_aligned(sizeof(fpu_state_t), FPU_STATE_ALIGNMENT);
    fpu_init_state(thread->fpu_state);
#endif
    return 0;
}

int thread_copy_of(thread_t* thread, thread_t* from_thread)
{
    memcpy(thread->tf, from_thread->tf, sizeof(trapframe_t));
    memcpy(thread->signal_handlers, from_thread->signal_handlers, sizeof(from_thread->signal_handlers));
#ifdef FPU_ENABLED
    memcpy(thread->fpu_state, from_thread->fpu_state, sizeof(fpu_state_t));
#endif
    return 0;
}

/**
 * STACK FUNCTIONS
 */

int thread_fill_up_stack(thread_t* thread, int argc, char** argv, int envp_count, char** envp)
{
    ASSERT(false);
    const int alignment = sizeof(uintptr_t);

    size_t argv_data_size = 0;
    for (int i = 0; i < argc; i++) {
        argv_data_size += strlen(argv[i]) + 1;
    }

    if (argv_data_size % alignment) {
        argv_data_size += alignment - (argv_data_size % alignment);
    }

    size_t envp_data_size = 0;
    for (int i = 0; i < envp_count; i++) {
        envp_data_size += strlen(envp[i]) + 1;
    }

    if (envp_data_size % alignment) {
        envp_data_size += alignment - (envp_data_size % alignment);
    }

    const size_t envp_array_size = (envp_count + 1) * sizeof(char*);
    const size_t argv_array_size = (argc + 1) * sizeof(char*);

#ifdef __i386__
    const size_t pointers_size = sizeof(argc) + sizeof(char*) + sizeof(char*); // argc + pointer to argv array + pointer to envp array.
#elif __arm__
    const size_t pointers_size = 0;
#elif __aarch64__
    const size_t pointers_size = 0;
#endif
    const size_t arrays_size = argv_array_size + envp_array_size;
    const size_t data_size = argv_data_size + envp_data_size;
    const size_t total_size_on_stack = data_size + arrays_size + pointers_size;
    int* tmp_buf = (int*)kmalloc(total_size_on_stack);
    if (!tmp_buf) {
        return -EAGAIN;
    }
    memset((void*)tmp_buf, 0, total_size_on_stack);

    // Resolve pointers from the start of stack
    char* tmp_buf_ptr = ((char*)tmp_buf) + total_size_on_stack;
    char* tmp_buf_envp_data_ptr = tmp_buf_ptr - envp_data_size;
    uintptr_t* tmp_buf_envp_array_ptr = (uintptr_t*)((char*)tmp_buf_envp_data_ptr - envp_array_size);
    char* tmp_buf_argv_data_ptr = (char*)tmp_buf_envp_array_ptr - argv_data_size;
    uintptr_t* tmp_buf_argv_array_ptr = (uintptr_t*)((char*)tmp_buf_argv_data_ptr - argv_array_size);
    int* tmp_buf_envp_ptr = (int*)((char*)tmp_buf_argv_array_ptr - sizeof(char*));
    int* tmp_buf_argv_ptr = (int*)((char*)tmp_buf_envp_ptr - sizeof(char*));
    int* tmp_buf_argc_ptr = (int*)((char*)tmp_buf_argv_ptr - sizeof(int));

    uintptr_t envp_data_sp = get_stack_pointer(thread->tf) - envp_data_size;
    uintptr_t envp_array_sp = envp_data_sp - envp_array_size;
    uintptr_t argv_data_sp = envp_array_sp - argv_data_size;
    uintptr_t argv_array_sp = argv_data_sp - argv_array_size;
#ifdef __i386__
    uintptr_t envp_sp = argv_array_sp - sizeof(char*);
    uintptr_t argv_sp = envp_sp - sizeof(char*);
    uintptr_t argc_sp = argv_sp - sizeof(int);
    uintptr_t end_sp = argc_sp;
    uintptr_t copy_to_sp = end_sp;
#elif __arm__
    uintptr_t end_sp = argv_array_sp;
    uintptr_t copy_to_sp = end_sp;

    // The alignment of sp must be 2x of the pointer size.
    end_sp = end_sp & ~(uintptr_t)(alignment * 2 - 1);
#elif __aarch64__
    uintptr_t end_sp = argv_array_sp;
    uintptr_t copy_to_sp = end_sp;

    // The alignment of sp must be 2x of the pointer size.
    end_sp = end_sp & ~(uintptr_t)(alignment * 2 - 1);
#endif

    // Fill argv
    char* top_of_argv_data = tmp_buf_argv_data_ptr + argv_data_size;
    set_stack_pointer(thread->tf, argv_data_sp + argv_data_size);
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]);
        top_of_argv_data -= len + 1;
        tf_move_stack_pointer(thread->tf, -(len + 1));
        memcpy(top_of_argv_data, argv[i], len);
        top_of_argv_data[len] = 0;

        tmp_buf_argv_array_ptr[i] = get_stack_pointer(thread->tf);
    }
    tmp_buf_argv_array_ptr[argc] = 0;

    // Fill envp
    char* top_of_envp_data = tmp_buf_envp_data_ptr + envp_data_size;
    set_stack_pointer(thread->tf, envp_data_sp + envp_data_size);
    for (int i = envp_count - 1; i >= 0; i--) {
        size_t len = strlen(envp[i]);
        top_of_envp_data -= len + 1;
        tf_move_stack_pointer(thread->tf, -(len + 1));
        memcpy(top_of_envp_data, envp[i], len);
        top_of_envp_data[len] = 0;

        tmp_buf_envp_array_ptr[i] = get_stack_pointer(thread->tf);
    }
    tmp_buf_envp_array_ptr[envp_count] = 0;

#ifdef __i386__
    *tmp_buf_envp_ptr = envp_array_sp;
    *tmp_buf_argv_ptr = argv_array_sp;
    *tmp_buf_argc_ptr = argc;
#elif __arm__
    thread->tf->r[0] = argc;
    thread->tf->r[1] = argv_array_sp;
    thread->tf->r[2] = envp_array_sp;
#elif __aarch64__
    thread->tf->x[0] = argc;
    thread->tf->x[1] = argv_array_sp;
    thread->tf->x[2] = envp_array_sp;
#endif
    set_stack_pointer(thread->tf, end_sp);
    vmm_copy_to_address_space(thread->process->address_space, (uint8_t*)tmp_buf, copy_to_sp, total_size_on_stack);
    kfree(tmp_buf);
    return 0;
}

int thread_kstack_free(thread_t* thread)
{
    kmemzone_free(thread->kstack);
#ifdef FPU_ENABLED
    kfree_aligned(thread->fpu_state);
#endif
    return 0;
}

int thread_free(thread_t* thread)
{
    if (thread->status != THREAD_STATUS_DYING) {
        return -EINVAL;
    }

    thread_kstack_free(thread);
    thread->status = THREAD_STATUS_INVALID;
    return 0;
}

int thread_die(thread_t* thread)
{
    if (thread_is_freed(thread)) {
        return -EINVAL;
    }

    thread->status = THREAD_STATUS_DYING;
    sched_dequeue(thread);
    return 0;
}

int thread_stop(thread_t* thread)
{
    thread->status = THREAD_STATUS_STOPPED;
    sched_dequeue(thread);
    return 0;
}

int thread_stop_and_resched(thread_t* thread)
{
    thread->status = THREAD_STATUS_STOPPED;
    sched_dequeue(thread);
    resched();
    return 0;
}

int thread_continue(thread_t* thread)
{
    thread->status = THREAD_STATUS_RUNNING;
    return 0;
}

int thread_init_blocker(thread_t* thread, const blocker_t* blocker)
{
    thread->status = THREAD_STATUS_BLOCKED;
    thread->blocker.reason = blocker->reason;
    thread->blocker.should_unblock = blocker->should_unblock;
    thread->blocker.should_unblock_for_signal = blocker->should_unblock_for_signal;
    sched_dequeue(thread);
    return 0;
}

/**
 * DEBUG FUNCTIONS
 */

int thread_dump_frame(thread_t* thread)
{
#ifdef __i386__
    for (uintptr_t i = thread->tf->esp; i < thread->tf->ebp; i++) {
        uint8_t byte = *(uint8_t*)i;
        uintptr_t b32 = (uintptr_t)byte;
        log("%x - %x\n", i, b32);
    }
#endif
    return 0;
}