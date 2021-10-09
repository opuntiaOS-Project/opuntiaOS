/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
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
    thread->last_cpu = LAST_CPU_NOT_SET;

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
#ifdef FPU_ENABLED
    /* setting fpu */
    thread->fpu_state = kmalloc_aligned(sizeof(fpu_state_t), 16);
    fpu_init_state(thread->fpu_state);
#endif
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
    thread->last_cpu = LAST_CPU_NOT_SET;

    /* setting signal handlers to 0 */
    thread->signals_mask = 0xffffffff; /* for now all signals are legal */
    thread->pending_signals_mask = 0;
    memset((void*)thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    _thread_setup_kstack(thread, thread->kstack.start + VMM_PAGE_SIZE);
    tf_setup_as_user_thread(thread->tf);
#ifdef FPU_ENABLED
    /* setting fpu */
    thread->fpu_state = kmalloc_aligned(sizeof(fpu_state_t), 16);
    fpu_init_state(thread->fpu_state);
#endif
    return 0;
}

int thread_copy_of(thread_t* thread, thread_t* from_thread)
{
    memcpy(thread->tf, from_thread->tf, sizeof(trapframe_t));
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
    uint32_t argv_data_size = 0;
    for (int i = 0; i < argc; i++) {
        argv_data_size += strlen(argv[i]) + 1;
    }

    if (argv_data_size % 4) {
        argv_data_size += 4 - (argv_data_size % 4);
    }

    uint32_t envp_data_size = 0;
    for (int i = 0; i < envp_count; i++) {
        envp_data_size += strlen(envp[i]) + 1;
    }

    if (envp_data_size % 4) {
        envp_data_size += 4 - (envp_data_size % 4);
    }

    const uint32_t envp_array_size = (envp_count + 1) * sizeof(char*);
    const uint32_t argv_array_size = (argc + 1) * sizeof(char*);

#ifdef __i386__
    const uint32_t pointers_size = sizeof(argc) + sizeof(char*) + sizeof(char*); // argc + pointer to argv array + pointer to envp array.
#elif __arm__
    const uint32_t pointers_size = 0;
#endif
    const uint32_t arrays_size = argv_array_size + envp_array_size;
    const uint32_t data_size = argv_data_size + envp_data_size;
    const uint32_t total_size_on_stack = data_size + arrays_size + pointers_size;
    int* tmp_buf = (int*)kmalloc(total_size_on_stack);
    if (!tmp_buf) {
        return -EAGAIN;
    }
    memset((void*)tmp_buf, 0, total_size_on_stack);

    // Resolve pointers from the start of stack
    char* tmp_buf_ptr = ((char*)tmp_buf) + total_size_on_stack;
    char* tmp_buf_envp_data_ptr = tmp_buf_ptr - envp_data_size;
    uint32_t* tmp_buf_envp_array_ptr = (uint32_t*)((char*)tmp_buf_envp_data_ptr - envp_array_size);
    char* tmp_buf_argv_data_ptr = (char*)tmp_buf_envp_array_ptr - argv_data_size;
    uint32_t* tmp_buf_argv_array_ptr = (uint32_t*)((char*)tmp_buf_argv_data_ptr - argv_array_size);
    int* tmp_buf_envp_ptr = (int*)((char*)tmp_buf_argv_array_ptr - sizeof(char*));
    int* tmp_buf_argv_ptr = (int*)((char*)tmp_buf_envp_ptr - sizeof(char*));
    int* tmp_buf_argc_ptr = (int*)((char*)tmp_buf_argv_ptr - sizeof(int));

    uint32_t envp_data_sp = get_stack_pointer(thread->tf) - envp_data_size;
    uint32_t envp_array_sp = envp_data_sp - envp_array_size;
    uint32_t argv_data_sp = envp_array_sp - argv_data_size;
    uint32_t argv_array_sp = argv_data_sp - argv_array_size;
#ifdef __i386__
    uint32_t envp_sp = argv_array_sp - sizeof(char*);
    uint32_t argv_sp = envp_sp - sizeof(char*);
    uint32_t argc_sp = argv_sp - sizeof(int);
    uint32_t end_sp = argc_sp;
#elif __arm__
    uint32_t end_sp = argv_array_sp;
#endif

    // Fill argv
    char* top_of_argv_data = tmp_buf_argv_data_ptr + argv_data_size;
    set_stack_pointer(thread->tf, argv_data_sp + argv_data_size);
    for (int i = argc - 1; i >= 0; i--) {
        uint32_t len = strlen(argv[i]);
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
        uint32_t len = strlen(envp[i]);
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
#endif
    set_stack_pointer(thread->tf, end_sp);

    vmm_copy_to_pdir(thread->process->pdir, (uint8_t*)tmp_buf, get_stack_pointer(thread->tf), total_size_on_stack);
    kfree(tmp_buf);
    return 0;
}

int thread_kstack_free(thread_t* thread)
{
    zoner_free_zone(thread->kstack);
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
    thread->status = THREAD_STATUS_DEAD;
    return 0;
}

int thread_die(thread_t* thread)
{
    if (thread_is_free(thread)) {
        return -EINVAL;
    }

    // If the thread was blocked, it could have a relation to a joinee thread. Break it here.
    if (thread->status == THREAD_STATUS_BLOCKED && thread->blocker.reason == BLOCKER_JOIN && thread->blocker_data.join.joinee) {
        thread_dec_waiting_ents(thread->blocker_data.join.joinee);
    }

    thread->status = THREAD_STATUS_DYING;
    sched_dequeue(thread);
    return 0;
}

int thread_zombie(thread_t* thread)
{
    if (thread_is_free(thread)) {
        return -EINVAL;
    }

    thread->status = THREAD_STATUS_ZOMBIE;
    sched_dequeue(thread);
    return 0;
}

int thread_dec_waiting_ents(thread_t* thread)
{
    int val = atomic_add(&thread->waiting_threads, -1);
    if (thread->status == THREAD_STATUS_ZOMBIE && val == 0) {
        thread->status = THREAD_STATUS_DYING;
    }
    if (thread->process->status == PROC_ZOMBIE) {
        proc_can_zombie_die(thread->process);
    }

    return val;
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