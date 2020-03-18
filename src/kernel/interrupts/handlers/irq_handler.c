#include <irq_handler.h>
#include <tasking/tasking.h>

void irq_redirect(uint8_t int_no) {
    void (*func)() = (void*)handlers[int_no];
    func();
}

void irq_handler(trapframe_t *tf) {
    if (tf->int_no >= IRQ_SLAVE_OFFSET) {
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);
    irq_redirect(tf->int_no);
}

void irq_empty_handler() {}