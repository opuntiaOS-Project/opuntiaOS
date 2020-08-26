#include <isr_handler.h>
#include <mem/vmm/vmm.h>
#include <tasking/tasking.h>
#include <tasking/sched.h>
#include <utils/kassert.h>
#include <x86/common.h>

void isr_handler(trapframe_t* tf)
{
    isr_standart_handler(tf);
}

void isr_standart_handler(trapframe_t* tf)
{
    cli();
    const char* exception_messages[32] = { "Division by zero",
        "Debug",
        "Non-maskable interrupt",
        "Breakpoint",
        "Detected overflow",
        "Out-of-bounds",
        "Invalid opcode", //Invalid opcode
        "No coprocessor",
        "Double fault",
        "Coprocessor segment overrun",
        "Bad TSS",
        "Segment not present",
        "Stack fault",
        "General protection fault", // General protection fault
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
        "Reserved" };

    /* TODO: A thing to change */
    if (tf->int_no == 14) {
        int res = vmm_page_fault_handler(tf->err, read_cr2());
        if (res == SHOULD_CRASH) {
            proc_t* p = tasking_get_active_proc();
            kprintf("\nCrash %d\n", p->pid);
            tasking_die(p);
            presched();
        }
    } else if (tf->int_no == 6) {
        kprintf("invalid opcode ");
        proc_t* proc = tasking_get_active_proc();
        if (proc == 0) {
            kpanic("in kernel\n");
        } else {
            kprintf("in proc %d\n", proc->pid);
        }
        while (1) { }
    } else {
        kprintf("INT %d: %s: %d", tf->int_no, exception_messages[tf->int_no], tf->err);
    }
    sti();
}
