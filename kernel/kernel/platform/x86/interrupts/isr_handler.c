#include <libkern/kassert.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <platform/generic/registers.h>
#include <platform/generic/system.h>
#include <platform/x86/isr_handler.h>
#include <tasking/dump.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

static char err_buf[64];

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

void isr_handler(trapframe_t* tf)
{
    system_disable_interrupts();

    proc_t* p = NULL;
    if (likely(RUNNIG_THREAD)) {
        p = RUNNIG_THREAD->process;
        if (RUNNIG_THREAD->process->is_kthread) {
            RUNNIG_THREAD->tf = tf;
        }
    }

    /* TODO: A thing to change */
    if (tf->int_no == 14) {
        int res = vmm_page_fault_handler(tf->err, read_cr2());
        if (res == SHOULD_CRASH) {
            if (!p) {
                snprintf(err_buf, 64, "Kernel trap at %x, type %d=%s", tf->eip, tf->int_no, &exception_messages[tf->int_no]);
                kpanic_tf(err_buf, tf);
            } else {
                log_warn("Crash: pf err %d at %x: %d pid, %x eip\n", tf->err, 0, p->pid, tf->eip);
                dump_and_kill(p);
            }
        }
    } else if (tf->int_no == 6) {
        if (!p) {
            snprintf(err_buf, 64, "Kernel trap at %x, type %d=%s", tf->eip, tf->int_no, &exception_messages[tf->int_no]);
            kpanic_tf(err_buf, tf);
        } else {
            log_warn("Crash: invalid opcode in %d tid\n", RUNNIG_THREAD->tid);
            dump_and_kill(p);
        }
    } else {
        log_warn("Int w/o handler: %d: %s: %d", tf->int_no, exception_messages[tf->int_no], tf->err);
    }

    /* We are leaving interrupt, and later interrupts will be on,
       when flags are restored */
    system_enable_interrupts_only_counter();
}
