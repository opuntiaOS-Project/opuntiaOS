/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tasking/thread.h>
#include <x86/gdt.h>
#include <x86/tss.h>

extern void trap_return();
extern void _tasking_jumper();

int _thread_setup_kstack(thread_t* thread)
{
    char* sp = (char*)(thread->kstack.start + VMM_PAGE_SIZE);

    /* setting trapframe in kernel stack */
    sp -= sizeof(*thread->tf);
    thread->tf = (trapframe_t*)sp;

    /* setting return point in kernel stack */
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)trap_return;

    /* setting context in kernel stack */
    sp -= sizeof(*thread->context);
    thread->context = (context_t*)sp;

    /* setting init data */
    memset((void*)thread->context, 0, sizeof(*thread->context));
    thread->context->eip = (uint32_t)_tasking_jumper;
    memset((void*)thread->tf, 0, sizeof(*thread->tf));

    return 0;
}

static void _thread_setup_segment_regs(thread_t* thread)
{
    thread->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    thread->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    thread->tf->es = thread->tf->ds;
    thread->tf->ss = thread->tf->ds;
    thread->tf->eflags = FL_IF;
}

void thread_set_stack(thread_t* thread, uint32_t ebp, uint32_t esp)
{
    thread->tf->ebp = ebp;
    thread->tf->esp = esp;
}

void thread_set_eip(thread_t* thread, uint32_t eip)
{
    thread->tf->eip = eip;
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
    _thread_setup_kstack(thread);
    _thread_setup_segment_regs(thread);
    return 0;
}

/**
 * STACK FUNCTIONS
 */

static inline void _thread_push_to_user_stack(thread_t* thread, uint32_t value)
{
    thread->tf->esp -= 4;
    *((uint32_t*)thread->tf->esp) = value;
}

static inline uint32_t _thread_pop_from_user_stack(thread_t* thread)
{
    uint32_t val = *((uint32_t*)thread->tf->esp);
    thread->tf->esp += 4;
    return val;
}

static inline void _thread_simulate_push_to_user_stack(thread_t* thread)
{
    thread->tf->esp -= 4;
}

static inline void _thread_simulate_pop_from_user_stack(thread_t* thread)
{
    thread->tf->esp += 4;
}

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

    uint32_t size_in_stack = argv_data_size + (argc + 1) * sizeof(char*) + sizeof(argc) + sizeof(char*);
    int* tmp_buf = (int*)kmalloc(size_in_stack);
    if (!tmp_buf) {
        return -EAGAIN;
    }
    memset((void*)tmp_buf, 0, size_in_stack);

    char* tmp_buf_ptr = ((char*)tmp_buf) + size_in_stack;
    char* tmp_buf_data_ptr = tmp_buf_ptr - argv_data_size;
    uint32_t* tmp_buf_array_ptr = (uint32_t*)((char*)tmp_buf_data_ptr - (argc + 1) * sizeof(char*));
    int* tmp_buf_argv_ptr = (int*)((char*)tmp_buf_array_ptr - sizeof(char*));
    int* tmp_buf_argc_ptr = (int*)((char*)tmp_buf_argv_ptr - sizeof(int));

    uint32_t data_esp = thread->tf->esp - argv_data_size;
    uint32_t array_esp = data_esp - (argc + 1) * sizeof(char*);
    uint32_t argv_esp = array_esp - 4;
    uint32_t argc_esp = argv_esp - 4;

    for (int i = argc - 1; i >= 0; i--) {
        uint32_t len = strlen(argv[i]);
        tmp_buf_ptr -= len + 1;
        thread->tf->esp -= len + 1;
        memcpy(tmp_buf_ptr, argv[i], len);
        tmp_buf_ptr[len] = 0;

        tmp_buf_array_ptr[i] = thread->tf->esp;
    }
    tmp_buf_array_ptr[argc] = 0;

    *tmp_buf_argv_ptr = array_esp;
    *tmp_buf_argc_ptr = argc;

    thread->tf->esp = argc_esp;

    vmm_copy_to_pdir(thread->process->pdir, (uint8_t*)tmp_buf, thread->tf->esp, size_in_stack);

    kfree(tmp_buf);

    return 0;
}

int thread_free(thread_t* thread)
{
    thread->status = THREAD_DEAD;
    zoner_free_zone(thread->kstack);
    return 0;
}

int thread_die(thread_t* thread)
{
    thread->status = THREAD_DYING;
    return 0;
}

/**
 * DEBUG FUNCTIONS
 */

int thread_dump_frame(thread_t* thread)
{
    for (uint32_t i = thread->tf->esp; i < thread->tf->ebp; i++) {
        uint8_t byte = *(uint8_t*)i;
        uint32_t b32 = (uint32_t)byte;
        kprintf("%x - %x\n", i, b32);
    }
}
