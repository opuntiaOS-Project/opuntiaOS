/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/kassert.h>
#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/generic/registers.h>
#include <platform/generic/system.h>
#include <platform/x86/fpu/fpu.h>
#include <platform/x86/isr_handler.h>
#include <tasking/cpu.h>
#include <tasking/dump.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

#define ERR_BUF_SIZE 64
static char err_buf[ERR_BUF_SIZE];

static const char* exception_messages[32] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Detected overflow",
    "Out-of-bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void x86_process_tf_for_kthread(trapframe_t* tf)
{
    if (likely(RUNNING_THREAD)) {
        if (RUNNING_THREAD->process->is_kthread && cpu_get_state() == CPU_IN_USERLAND) {
            RUNNING_THREAD->tf = tf;
        }
    }
}

void isr_handler(trapframe_t* frame)
{
#ifdef PREEMPT_KERNEL
    system_enable_interrupts_no_counter();
#else
    system_disable_interrupts();
#endif
    x86_process_tf_for_kthread(frame);
    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();

    int res;
    proc_t* proc = NULL;
    if (likely(RUNNING_THREAD)) {
        proc = RUNNING_THREAD->process;
    }

    switch (frame->int_no) {
    /* Division by 0 or kernel trap (if no process). */
    case 0:
        if (proc) {
            log_warn("Crash: division by zero in T%d\n", RUNNING_THREAD->tid);
            dump_and_kill(proc);
        } else {
            snprintf(
                err_buf, ERR_BUF_SIZE, "Kernel trap at %x, type %d=%s",
                frame->eip, frame->int_no,
                &exception_messages[frame->int_no]);
            kpanic_tf(err_buf, frame);
        }
        break;

    /* Debug, non-maskable interrupt, breakpoint, detected overflow,
           out of bounds. */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        log_error("Int w/o handler: %d: %s: %d", frame->int_no,
            exception_messages[frame->int_no], frame->err);
        system_stop();
        break;

    /* Invalid opcode or kernel trap (if no process). */
    case 6:
        if (proc) {
            log_warn("Crash: invalid opcode in %d tid\n", RUNNING_THREAD->tid);
            dump_and_kill(proc);
        } else {
            snprintf(
                err_buf, ERR_BUF_SIZE, "Kernel trap at %x, type %d=%s",
                frame->eip, frame->int_no, &exception_messages[frame->int_no]);
            kpanic_tf(err_buf, frame);
        }
        break;

    /* No coprocessor */
    case 7:
        fpu_handler();
        break;

    /* Double fault and other. */
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        log_error("Int w/o handler: %d: %s: %d", frame->int_no,
            exception_messages[frame->int_no], frame->err);
        system_stop();
        break;

    case 14:
        res = vmm_page_fault_handler(frame->err, read_cr2());
        if (res == 0)
            break;

        if (proc) {
            log_warn("Crash: pf err %d at %x: %d pid, %x eip",
                frame->err, read_cr2(), proc->pid, frame->eip);
            dump_and_kill(proc);
        } else {
            snprintf(
                err_buf, ERR_BUF_SIZE, "Kernel trap at %x, type %d=%s",
                frame->eip, frame->int_no, &exception_messages[frame->int_no]);
            kpanic_tf(err_buf, frame);
        }
        break;

    case 15:
        log_error("Int w/o handler: %d: %s: %d", frame->int_no,
            exception_messages[frame->int_no], frame->err);
        system_stop();
        break;
    default:
        log_error("Int w/o handler: %d: %d", frame->int_no, frame->err);
        system_stop();
    }

    cpu_set_state(prev_cpu_state);
#ifndef PREEMPT_KERNEL
    // When we are leaving the interrupt handler, we want to jump back into
    // user space and enable the x86 PIC again.
    system_enable_interrupts_only_counter();
#endif
}
