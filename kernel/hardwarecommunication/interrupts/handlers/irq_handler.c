#include <irq_handler.h>
#include <display.h>

void irq_handler_redirect(regs_t regs) {
    void (*func)(regs_t regs) = handlers[regs.int_no];
    func(regs);
}

void irq_handler_master(regs_t regs) {
    irq_handler_redirect(regs);
    port_byte_out(0x20, 0x20);
}

void irq_handler_slave(regs_t regs) {
    irq_handler_redirect(regs);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20);
}

void irq_handler_null(regs_t regs) { // standart empty handler
    // printd(regs.int_no); 
    // printf("\n");
}