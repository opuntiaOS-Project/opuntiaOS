/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/generic/system.h>
#include <platform/x86/irq_handler.h>
#include <tasking/cpu.h>
#include <tasking/tasking.h>

extern void x86_process_tf_for_kthread(trapframe_t* tf);

static inline void irq_redirect(uint8_t int_no)
{
    void (*func)() = (void*)handlers[int_no];
    func();
}

static void irq_accept_next(int int_no)
{
    if (int_no >= IRQ_SLAVE_OFFSET) {
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);
}

void irq_handler(trapframe_t* tf)
{
#ifdef PREEMPT_KERNEL
    system_enable_interrupts_no_counter();
#else
    system_disable_interrupts();
#endif
    x86_process_tf_for_kthread(tf);
    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();

    switch (tf->int_no) {
    case 1:
        // Since the timer handler could call resched(), it is needed
        // to reset irq before calling the handler.
        irq_accept_next(tf->int_no);
        irq_redirect(tf->int_no);

    default:
        irq_redirect(tf->int_no);
        irq_accept_next(tf->int_no);
    }

    cpu_set_state(prev_cpu_state);
#ifndef PREEMPT_KERNEL
    // We are leaving interrupt, and later interrupts will be on,
    // when flags are restored.
    system_enable_interrupts_only_counter();
#endif
}

void irq_empty_handler()
{
    return;
}