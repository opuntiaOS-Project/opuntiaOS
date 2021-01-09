#include <platform/x86/isr_handler.h>
#include <mem/vmm/vmm.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>
#include <utils/kassert.h>
#include <platform/x86/registers.h>
#include <log.h>

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
    disable_intrs();
    
    proc_t* p = 0; 
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
            log_warn("Crash: pf err %d at %x: %d pid, %x eip\n", tf->err, read_cr2(), p->pid, tf->eip);
            thread_print_backtrace();
            proc_die(p);
            resched();
        }
    } else if (tf->int_no == 6) {
        if (!p) {
            kpanic("invalid opcode in kernel");
        } else {
            log_warn("Crash: invalid opcode in %d tid\n", RUNNIG_THREAD->tid);
            thread_print_backtrace();
            proc_die(p);
            resched();
        }
    } else {
        log_warn("Int w/o handler: %d: %s: %d", tf->int_no, exception_messages[tf->int_no], tf->err);
    }

    /* We are leaving interrupt, and later interrupts will be on,
       when flags are restored */
    enable_intrs_only_counter();
}
