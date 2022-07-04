/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <libkern/mem.h>
#include <platform/x86/fpu/fpu.h>
#include <platform/x86/idt.h>
#include <tasking/tasking.h>

static fpu_state_t fpu_state;

#define DEBUG_FPU

void fpu_setup(void)
{
    uintptr_t tmp;
    asm volatile("mov %%cr0, %0"
                 : "=r"(tmp));
    tmp &= ~(1 << 2);
    tmp |= (1 << 1);
    asm volatile("mov %0, %%cr0" ::"r"(tmp));

    asm volatile("mov %%cr4, %0"
                 : "=r"(tmp));
    tmp |= 3 << 9;
    asm volatile("mov %0, %%cr4" ::"r"(tmp));
}

void fpu_handler()
{
    if (!RUNNING_THREAD) {
#ifdef DEBUG_FPU
        log_warn("FPU: no running thread, but handler is called");
#endif
        return;
    }

    if (fpu_is_avail()) {
#ifdef DEBUG_FPU
        log_warn("FPU: is avail, but handler is called");
#endif
        return;
    }

    fpu_make_avail();

    if (RUNNING_THREAD->tid == THIS_CPU->fpu_for_pid) {
        return;
    }

    if (THIS_CPU->fpu_for_thread && thread_is_alive(THIS_CPU->fpu_for_thread) && THIS_CPU->fpu_for_thread->tid == THIS_CPU->fpu_for_pid) {
        fpu_save(THIS_CPU->fpu_for_thread->fpu_state);
    }

    fpu_restore(RUNNING_THREAD->fpu_state);
    THIS_CPU->fpu_for_thread = RUNNING_THREAD;
    THIS_CPU->fpu_for_pid = RUNNING_THREAD->tid;
}

void fpu_init()
{
    fpu_setup();
    asm volatile("fninit");
    asm volatile("fxsave %0"
                 : "=m"(fpu_state));
}

void fpu_init_state(fpu_state_t* new_fpu_state)
{
    memcpy(new_fpu_state, &fpu_state, sizeof(fpu_state_t));
}