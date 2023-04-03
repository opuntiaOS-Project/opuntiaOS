/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/generic/tasking/signal_impl.h>
#include <tasking/thread.h>

int signal_impl_prepare_stack(thread_t* thread, int signo, uintptr_t old_sp, uintptr_t magic)
{
    tf_push_to_stack(thread->tf, thread->tf->ra);
    tf_push_to_stack(thread->tf, thread->tf->sp);
    tf_push_to_stack(thread->tf, thread->tf->gp);
    tf_push_to_stack(thread->tf, thread->tf->tp);
    tf_push_to_stack(thread->tf, thread->tf->t0);
    tf_push_to_stack(thread->tf, thread->tf->t1);
    tf_push_to_stack(thread->tf, thread->tf->t2);
    tf_push_to_stack(thread->tf, thread->tf->s0);
    tf_push_to_stack(thread->tf, thread->tf->s1);
    tf_push_to_stack(thread->tf, thread->tf->a0);
    tf_push_to_stack(thread->tf, thread->tf->a1);
    tf_push_to_stack(thread->tf, thread->tf->a2);
    tf_push_to_stack(thread->tf, thread->tf->a3);
    tf_push_to_stack(thread->tf, thread->tf->a4);
    tf_push_to_stack(thread->tf, thread->tf->a5);
    tf_push_to_stack(thread->tf, thread->tf->a6);
    tf_push_to_stack(thread->tf, thread->tf->a7);
    tf_push_to_stack(thread->tf, thread->tf->s2);
    tf_push_to_stack(thread->tf, thread->tf->s3);
    tf_push_to_stack(thread->tf, thread->tf->s4);
    tf_push_to_stack(thread->tf, thread->tf->s5);
    tf_push_to_stack(thread->tf, thread->tf->s6);
    tf_push_to_stack(thread->tf, thread->tf->s7);
    tf_push_to_stack(thread->tf, thread->tf->s8);
    tf_push_to_stack(thread->tf, thread->tf->s9);
    tf_push_to_stack(thread->tf, thread->tf->s10);
    tf_push_to_stack(thread->tf, thread->tf->s11);
    tf_push_to_stack(thread->tf, thread->tf->t3);
    tf_push_to_stack(thread->tf, thread->tf->t4);
    tf_push_to_stack(thread->tf, thread->tf->t5);
    tf_push_to_stack(thread->tf, thread->tf->t6);
    tf_push_to_stack(thread->tf, thread->tf->sstatus);
    tf_push_to_stack(thread->tf, thread->tf->epc);
    tf_push_to_stack(thread->tf, old_sp);
    tf_push_to_stack(thread->tf, magic);

    thread->tf->a0 = signo;
    thread->tf->a1 = (uintptr_t)thread->signal_handlers[signo];
    return 0;
}

int signal_impl_restore_stack(thread_t* thread, uintptr_t* old_sp, uintptr_t* magic)
{
    *magic = tf_pop_to_stack(thread->tf);
    *old_sp = tf_pop_to_stack(thread->tf);
    thread->tf->epc = tf_pop_to_stack(thread->tf);
    thread->tf->sstatus = tf_pop_to_stack(thread->tf);
    thread->tf->t6 = tf_pop_to_stack(thread->tf);
    thread->tf->t5 = tf_pop_to_stack(thread->tf);
    thread->tf->t4 = tf_pop_to_stack(thread->tf);
    thread->tf->t3 = tf_pop_to_stack(thread->tf);
    thread->tf->s11 = tf_pop_to_stack(thread->tf);
    thread->tf->s10 = tf_pop_to_stack(thread->tf);
    thread->tf->s9 = tf_pop_to_stack(thread->tf);
    thread->tf->s8 = tf_pop_to_stack(thread->tf);
    thread->tf->s7 = tf_pop_to_stack(thread->tf);
    thread->tf->s6 = tf_pop_to_stack(thread->tf);
    thread->tf->s5 = tf_pop_to_stack(thread->tf);
    thread->tf->s4 = tf_pop_to_stack(thread->tf);
    thread->tf->s3 = tf_pop_to_stack(thread->tf);
    thread->tf->s2 = tf_pop_to_stack(thread->tf);
    thread->tf->a7 = tf_pop_to_stack(thread->tf);
    thread->tf->a6 = tf_pop_to_stack(thread->tf);
    thread->tf->a5 = tf_pop_to_stack(thread->tf);
    thread->tf->a4 = tf_pop_to_stack(thread->tf);
    thread->tf->a3 = tf_pop_to_stack(thread->tf);
    thread->tf->a2 = tf_pop_to_stack(thread->tf);
    thread->tf->a1 = tf_pop_to_stack(thread->tf);
    thread->tf->a0 = tf_pop_to_stack(thread->tf);
    thread->tf->s1 = tf_pop_to_stack(thread->tf);
    thread->tf->s0 = tf_pop_to_stack(thread->tf);
    thread->tf->t2 = tf_pop_to_stack(thread->tf);
    thread->tf->t1 = tf_pop_to_stack(thread->tf);
    thread->tf->t0 = tf_pop_to_stack(thread->tf);
    thread->tf->tp = tf_pop_to_stack(thread->tf);
    thread->tf->gp = tf_pop_to_stack(thread->tf);
    thread->tf->sp = tf_pop_to_stack(thread->tf);
    thread->tf->ra = tf_pop_to_stack(thread->tf);
    return 0;
}