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
    for (int i = 0; i < 13; i++) {
        tf_push_to_stack(thread->tf, thread->tf->r[i]);
    }
    tf_push_to_stack(thread->tf, old_sp);
    tf_push_to_stack(thread->tf, thread->tf->user_sp);
    tf_push_to_stack(thread->tf, thread->tf->user_ip);
    tf_push_to_stack(thread->tf, thread->tf->user_flags);
    tf_push_to_stack(thread->tf, magic);
    thread->tf->r[0] = signo;
    thread->tf->r[1] = (uintptr_t)thread->signal_handlers[signo];
    return 0;
}

int signal_impl_restore_stack(thread_t* thread, uintptr_t* old_sp, uintptr_t* magic)
{
    *magic = tf_pop_to_stack(thread->tf);
    thread->tf->user_flags = tf_pop_to_stack(thread->tf);
    thread->tf->user_ip = tf_pop_to_stack(thread->tf);
    thread->tf->user_sp = tf_pop_to_stack(thread->tf);
    *old_sp = tf_pop_to_stack(thread->tf);
    for (int i = 12; i >= 0; i--) {
        thread->tf->r[i] = tf_pop_to_stack(thread->tf);
    }
    return 0;
}