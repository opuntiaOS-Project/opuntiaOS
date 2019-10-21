#include <irq_handler.h>

void irq_redirect(uint8_t int_no) {
    void (*func)() = handlers[int_no];
    func();
}

uint32_t irq_handler(uint8_t int_no, uint32_t esp) {
    irq_redirect(int_no);
    if (int_no >= IRQ_SLAVE_OFFSET) {
        port_byte_out(0xA0, 0x20);
    }
    port_byte_out(0x20, 0x20);
    return esp;
}

void irq_empty_handler() {}