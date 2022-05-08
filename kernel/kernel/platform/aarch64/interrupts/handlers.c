/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch64/gicv2.h>
#include <drivers/aarch64/timer.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/aarch64/interrupts.h>
#include <platform/aarch64/registers.h>
#include <platform/aarch64/system.h>
#include <platform/aarch64/tasking/trapframe.h>
#include <syscalls/handlers.h>
#include <tasking/dump.h>
#include <tasking/sched.h>

#define ERR_BUF_SIZE 64
static char err_buf[ERR_BUF_SIZE];

static gic_descritptor_t gic_descriptor;
static irq_handler_t _irq_handlers[IRQ_HANDLERS_MAX];

static void _irq_empty_handler()
{
    return;
}

static void init_irq_handlers()
{
    for (int i = 0; i < IRQ_HANDLERS_MAX; i++) {
        _irq_handlers[i] = _irq_empty_handler;
    }
}

void interrupts_setup()
{
    system_disable_interrupts();
    system_enable_interrupts_only_counter(); // Reset counter
    extern void set_up_vector_table();
    set_up_vector_table();
    init_irq_handlers();
}

void serror_handler(trapframe_t* tf)
{
    log("serror_handler");
    system_stop();
}

void sync_handler(trapframe_t* tf)
{
    system_disable_interrupts();

    int trap_state = THIS_CPU->current_state;

    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();
    uint64_t fault_addr = tf->far;
    uint64_t esr = tf->esr;
    uint64_t esr_ec = (esr & 0xFC000000) >> 26;

    // Instruction or data faults
    if (esr_ec == 0b100101 || esr_ec == 0b100100 || esr_ec == 0b100000 || esr_ec == 0b100001) {
        int err = vmm_page_fault_handler(esr, fault_addr);
        if (err) {
            if (trap_state == CPU_IN_KERNEL || !RUNNING_THREAD) {
                snprintf(err_buf, ERR_BUF_SIZE, "Kernel trap at %zx, %zx prefetch_abort_handler", tf->elr, fault_addr);
                kpanic_tf(err_buf, tf);
            } else {
                log_warn("Crash: sync abort %zd at %zx: %d pid, %zx eip\n", esr, fault_addr, RUNNING_THREAD->tid, tf);
                dump_and_kill(RUNNING_THREAD->process);
            }
        }
    } else if (esr_ec == 0b000111) {
#ifdef FPU_ENABLED
        if (!RUNNING_THREAD) {
            goto undefined_h;
        }

        if (fpu_is_avail()) {
            goto undefined_h;
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
        system_enable_interrupts_only_counter();
        return;
#endif // FPU_ENABLED
    undefined_h:
        ASSERT(false);
    } else {
        log("sync_handler ip: %zx = %zx : %zx", tf->elr, fault_addr, esr_ec);
        while (1) { }
    }

    cpu_set_state(prev_cpu_state);
    system_enable_interrupts_only_counter();
}

void sync_handler_from_el0(trapframe_t* tf)
{
    uint64_t esr_ec = (tf->esr & 0xFC000000) >> 26;
    if (esr_ec == 0b010101) {
        sys_handler(tf);
        return;
    }

    sync_handler(tf);
}

void irq_handler(trapframe_t* tf)
{
    system_disable_interrupts();
    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();
    aarch64_timer_rearm();
    gic_descriptor.end_interrupt(30);
    cpu_tick();
    timeman_timer_tick();
    sched_tick();
    cpu_set_state(prev_cpu_state);
    system_enable_interrupts_only_counter();
}

void fast_irq_handler(trapframe_t* tf)
{
    // Apl target got it here
    system_disable_interrupts();
    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();
    aarch64_timer_rearm();
    cpu_tick();
    timeman_timer_tick();
    sched_tick();
    cpu_set_state(prev_cpu_state);
    system_enable_interrupts_only_counter();
}

void gic_setup()
{
    gicv2_install();
}

void gic_setup_secondary_cpu()
{
    gicv2_install_secondary_cpu();
}

void irq_set_gic_desc(gic_descritptor_t gic_desc)
{
    gic_descriptor = gic_desc;
}

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_flags_t flags, irq_handler_t func, int cpu_mask)
{
    _irq_handlers[line] = func;
    gic_descriptor.enable_irq(line, prior, flags, cpu_mask);
}