/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/irq/riscv/plic.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/riscv64/interrupts.h>
#include <platform/riscv64/registers.h>
#include <platform/riscv64/system.h>
#include <platform/riscv64/tasking/trapframe.h>
#include <syscalls/handlers.h>
#include <tasking/dump.h>
#include <tasking/sched.h>

#define ERR_BUF_SIZE 64
static char err_buf[ERR_BUF_SIZE];

static irqdev_descritptor_t plic_descriptor;
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

static inline void _irq_redirect(irq_line_t line)
{
    _irq_handlers[line](line);
}

void interrupts_setup()
{
    system_disable_interrupts();
    system_enable_interrupts_only_counter(); // Reset counter
    extern void set_up_vector_table();
    set_up_vector_table();
    init_irq_handlers();
}

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_flags_t flags, irq_handler_t func, int cpu_mask)
{
    _irq_handlers[line] = func;
    plic_descriptor.enable_irq(line, prior, flags, cpu_mask);
}

void irq_set_dev(irqdev_descritptor_t plic_desc)
{
    plic_descriptor = plic_desc;
}

void plic_setup()
{
    plic_install();
}

void handle_interrupt(trapframe_t* tf)
{
    system_disable_interrupts();
    int trap_state = THIS_CPU->current_state;
    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();

    uint64_t cause = read_scause();
    uint64_t fault_addr = read_stval();
    if (cause == 8 || cause == 9) {
        tf->epc += 4;
        // log("Syscall %d", tf->a7);
        sys_handler(tf);
    } else if (cause < 16) {
        int err = vmm_page_fault_handler(cause, fault_addr);
        if (err) {
            if (trap_state == CPU_IN_KERNEL || !RUNNING_THREAD) {
                snprintf(err_buf, ERR_BUF_SIZE, "Kernel trap at %zx, %zx prefetch_abort_handler", tf->epc, fault_addr);
                kpanic_tf(err_buf, tf);
            } else {
                log_warn("Crash: sync abort %zx at %zx: %d pid, %zx eip", tf->epc, fault_addr, RUNNING_THREAD->tid, tf);
                dump_and_kill(RUNNING_THREAD->process);
            }
        }
    } else if (cause == 0x8000000000000001L) {
        write_sip(read_sip() & ~2);
        cpu_tick();
        timeman_timer_tick();
        sched_tick();
    } else if (TEST_FLAG(cause, 0x8000000000000000) && (cause & 0xff) == 9) {
        uint32_t int_disc = plic_descriptor.interrupt_descriptor();
        if (!int_disc) {
            goto exit;
        }
        plic_descriptor.end_interrupt(int_disc);
        _irq_redirect(int_disc);
    } else {
        ASSERT(false && "unknown interrupt");
    }
exit:
    cpu_set_state(prev_cpu_state);
    system_enable_interrupts_only_counter();
}
