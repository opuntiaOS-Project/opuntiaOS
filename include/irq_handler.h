#include <idt_common_data.h>

void irq_handler_master(u_int8 interrupt_no);
void irq_handler_slave(u_int8 interrupt_no);
void irq_handler_redirect(u_int8 interrupt_no);

void irq_handler_null();