#include <irq_handler.h>
#include <tasking/tasking.h>
#include <x86/common.h>

void irq_redirect(uint8_t int_no) {
    void (*func)() = (void*)handlers[int_no];
    func();
}

void irq_handler(trapframe_t *tf) {
    cli();
    if (tf->int_no >= IRQ_SLAVE_OFFSET) {
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);

    if (likely(RUNNIG_THREAD)) {
        if (RUNNIG_THREAD->process->is_kthread) {
            RUNNIG_THREAD->tf = tf;
        }
    }

    irq_redirect(tf->int_no);
    /* We are leaving interrupt, and later interrupts will be on,
       when flags are restored */
    sti_only_counter();
}

void irq_empty_handler() {
    return;
}