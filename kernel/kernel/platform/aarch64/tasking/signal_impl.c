/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/generic/tasking/signal_impl.h>
#include <tasking/thread.h>

int signal_impl_prepare_stack(thread_t* thread, int signo, uintptr_t old_sp, uintptr_t magic)
{
    for (int i = 0; i < 31; i++) {
        tf_push_to_stack(thread->tf, thread->tf->x[i]);
    }
    tf_push_to_stack(thread->tf, old_sp);
    tf_push_to_stack(thread->tf, thread->tf->esr);
    tf_push_to_stack(thread->tf, thread->tf->elr);
    tf_push_to_stack(thread->tf, thread->tf->far);
    tf_push_to_stack(thread->tf, thread->tf->spsr);
    tf_push_to_stack(thread->tf, thread->tf->sp);
    tf_push_to_stack(thread->tf, magic);
    thread->tf->x[0] = signo;
    thread->tf->x[1] = (uintptr_t)thread->signal_handlers[signo];
    return 0;
}

int signal_impl_restore_stack(thread_t* thread, uintptr_t* old_sp, uintptr_t* magic)
{
    *magic = tf_pop_to_stack(thread->tf);
    thread->tf->sp = tf_pop_to_stack(thread->tf);
    thread->tf->spsr = tf_pop_to_stack(thread->tf);
    thread->tf->far = tf_pop_to_stack(thread->tf);
    thread->tf->elr = tf_pop_to_stack(thread->tf);
    thread->tf->esr = tf_pop_to_stack(thread->tf);
    *old_sp = tf_pop_to_stack(thread->tf);
    for (int i = 30; i >= 0; i--) {
        thread->tf->x[i] = tf_pop_to_stack(thread->tf);
    }
    return 0;
}