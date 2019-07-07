#include "irq_handler.h"

void irq_handler_redirect(u_int8 interrupt_no) {
    void (*func)(void) = handlers[interrupt_no];
    func();
}

void irq_handler_master(u_int8 interrupt_no) {
    irq_handler_redirect(interrupt_no);
    port_byte_out(0x20, 0x20);
}

void irq_handler_slave(u_int8 interrupt_no) {
    irq_handler_redirect(interrupt_no);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20);
}

void irq_handler_null() {
    print_char('E', WHITE_ON_BLACK, -1, -1);
}