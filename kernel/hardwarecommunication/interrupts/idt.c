#include <idt.h>
#include <port.h>
#include <pic.h>

void idt_setup() {
	idt_element_setup(0, (u_int32)isr0);
	idt_element_setup(1, (u_int32)isr1);
	idt_element_setup(2, (u_int32)isr2);
	idt_element_setup(3, (u_int32)isr3);
	idt_element_setup(4, (u_int32)isr4);
	idt_element_setup(5, (u_int32)isr5);
	idt_element_setup(6, (u_int32)isr6);
	idt_element_setup(7, (u_int32)isr7);
	idt_element_setup(8, (u_int32)isr8);
	idt_element_setup(9, (u_int32)isr9);
	idt_element_setup(10, (u_int32)isr10);
    idt_element_setup(11, (u_int32)isr11);
    idt_element_setup(12, (u_int32)isr12);
    idt_element_setup(13, (u_int32)isr13);
    idt_element_setup(14, (u_int32)isr14);
    idt_element_setup(15, (u_int32)isr15);
    idt_element_setup(16, (u_int32)isr16);
    idt_element_setup(17, (u_int32)isr17);
    idt_element_setup(18, (u_int32)isr18);
    idt_element_setup(19, (u_int32)isr19);
    idt_element_setup(20, (u_int32)isr20);
    idt_element_setup(21, (u_int32)isr21);
    idt_element_setup(22, (u_int32)isr22);
    idt_element_setup(23, (u_int32)isr23);
    idt_element_setup(24, (u_int32)isr24);
    idt_element_setup(25, (u_int32)isr25);
    idt_element_setup(26, (u_int32)isr26);
    idt_element_setup(27, (u_int32)isr27);
    idt_element_setup(28, (u_int32)isr28);
    idt_element_setup(29, (u_int32)isr29);
    idt_element_setup(30, (u_int32)isr30);
    idt_element_setup(31, (u_int32)isr31);

	pic_remap(IRQ_MASTER_OFFSET, IRQ_SLAVE_OFFSET);

	idt_element_setup(32, (u_int32)irq0);
    idt_element_setup(33, (u_int32)irq1);
    idt_element_setup(34, (u_int32)irq2);
    idt_element_setup(35, (u_int32)irq3);
    idt_element_setup(36, (u_int32)irq4);
    idt_element_setup(37, (u_int32)irq5);
    idt_element_setup(38, (u_int32)irq6);
    idt_element_setup(39, (u_int32)irq7);
    idt_element_setup(40, (u_int32)irq8);
    idt_element_setup(41, (u_int32)irq9);
    idt_element_setup(42, (u_int32)irq10);
    idt_element_setup(43, (u_int32)irq11);
    idt_element_setup(44, (u_int32)irq12);
    idt_element_setup(45, (u_int32)irq13);
    idt_element_setup(46, (u_int32)irq14);
    idt_element_setup(47, (u_int32)irq15);

    init_irq_handlers();

	idt_load();

}

void setup_irq_handler(u_int8 interrupt_no, void (*handler)(regs_t regs)) {
    handlers[interrupt_no] = handler;
}

void init_irq_handlers() {
    int i;
    for (i = IRQ_MASTER_OFFSET; i < IRQ_MASTER_OFFSET + 8; i++){
        handlers[i] = irq_handler_null;
    }
    for (i = IRQ_SLAVE_OFFSET; i < IRQ_SLAVE_OFFSET + 8; i++){
        handlers[i] = irq_handler_null;
    }
}

void idt_load() {
	idt_register.base = (u_int32)&idt;
	idt_register.limit = (u_int16)(IDT_ENTRIES * sizeof(struct IDT_Entry) - 1);
	__asm__("lidt (%0)" : : "r"(&idt_register));
}

void idt_element_setup(u_int8 n, u_int32 handler_addr) {
	idt[n].offset_lower = handler_addr & 0xffff;
	idt[n].segment = CODE_SEG;
	idt[n].zero = 0;
	idt[n].type = 0x8E;
	idt[n].offset_upper = handler_addr >> 16;
}