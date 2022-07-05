/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/generic/tasking/signal_impl.h>
#include <tasking/thread.h>

#define RED_ZONE_SIZE (128)

int signal_impl_prepare_stack(thread_t* thread, int signo, uintptr_t old_sp, uintptr_t magic)
{
    tf_move_stack_pointer(thread->tf, -RED_ZONE_SIZE);
    tf_push_to_stack(thread->tf, thread->tf->rflags);
    tf_push_to_stack(thread->tf, thread->tf->rip);
    tf_push_to_stack(thread->tf, thread->tf->rax);
    tf_push_to_stack(thread->tf, thread->tf->rbx);
    tf_push_to_stack(thread->tf, thread->tf->rcx);
    tf_push_to_stack(thread->tf, thread->tf->rdx);
    tf_push_to_stack(thread->tf, thread->tf->r8);
    tf_push_to_stack(thread->tf, thread->tf->r9);
    tf_push_to_stack(thread->tf, thread->tf->r10);
    tf_push_to_stack(thread->tf, thread->tf->r11);
    tf_push_to_stack(thread->tf, thread->tf->r12);
    tf_push_to_stack(thread->tf, thread->tf->r13);
    tf_push_to_stack(thread->tf, thread->tf->r14);
    tf_push_to_stack(thread->tf, thread->tf->r15);
    tf_push_to_stack(thread->tf, old_sp);
    tf_push_to_stack(thread->tf, thread->tf->rbp);
    tf_push_to_stack(thread->tf, thread->tf->rsi);
    tf_push_to_stack(thread->tf, thread->tf->rdi);
    tf_push_to_stack(thread->tf, magic);
    tf_push_to_stack(thread->tf, 0xfacea660); // fake return address

    thread->tf->rdi = signo;
    thread->tf->rsi = (uintptr_t)thread->signal_handlers[signo];
    return 0;
}

int signal_impl_restore_stack(thread_t* thread, uintptr_t* old_sp, uintptr_t* magic)
{
    tf_move_stack_pointer(thread->tf, sizeof(uintptr_t)); // cleaning fake return address
    *magic = tf_pop_to_stack(thread->tf);
    thread->tf->rdi = tf_pop_to_stack(thread->tf);
    thread->tf->rsi = tf_pop_to_stack(thread->tf);
    thread->tf->rbp = tf_pop_to_stack(thread->tf);
    *old_sp = tf_pop_to_stack(thread->tf);
    thread->tf->r15 = tf_pop_to_stack(thread->tf);
    thread->tf->r14 = tf_pop_to_stack(thread->tf);
    thread->tf->r13 = tf_pop_to_stack(thread->tf);
    thread->tf->r12 = tf_pop_to_stack(thread->tf);
    thread->tf->r11 = tf_pop_to_stack(thread->tf);
    thread->tf->r10 = tf_pop_to_stack(thread->tf);
    thread->tf->r9 = tf_pop_to_stack(thread->tf);
    thread->tf->r8 = tf_pop_to_stack(thread->tf);
    thread->tf->rdx = tf_pop_to_stack(thread->tf);
    thread->tf->rcx = tf_pop_to_stack(thread->tf);
    thread->tf->rbx = tf_pop_to_stack(thread->tf);
    thread->tf->rax = tf_pop_to_stack(thread->tf);
    thread->tf->rip = tf_pop_to_stack(thread->tf);
    thread->tf->rflags = tf_pop_to_stack(thread->tf);
    tf_move_stack_pointer(thread->tf, RED_ZONE_SIZE);
    return 0;
}