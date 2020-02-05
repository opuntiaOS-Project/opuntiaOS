#include <x86/idt.h>

void idt_setup() {
	idt_element_setup(0, (void*)isr0);
	idt_element_setup(1, (void*)isr1);
	idt_element_setup(2, (void*)isr2);
	idt_element_setup(3, (void*)isr3);
	idt_element_setup(4, (void*)isr4);
	idt_element_setup(5, (void*)isr5);
	idt_element_setup(6, (void*)isr6);
	idt_element_setup(7, (void*)isr7);
	idt_element_setup(8, (void*)isr8);
	idt_element_setup(9, (void*)isr9);
	idt_element_setup(10, (void*)isr10);
    idt_element_setup(11, (void*)isr11);
    idt_element_setup(12, (void*)isr12);
    idt_element_setup(13, (void*)isr13);
    idt_element_setup(14, (void*)isr14);
    idt_element_setup(15, (void*)isr15);
    idt_element_setup(16, (void*)isr16);
    idt_element_setup(17, (void*)isr17);
    idt_element_setup(18, (void*)isr18);
    idt_element_setup(19, (void*)isr19);
    idt_element_setup(20, (void*)isr20);
    idt_element_setup(21, (void*)isr21);
    idt_element_setup(22, (void*)isr22);
    idt_element_setup(23, (void*)isr23);
    idt_element_setup(24, (void*)isr24);
    idt_element_setup(25, (void*)isr25);
    idt_element_setup(26, (void*)isr26);
    idt_element_setup(27, (void*)isr27);
    idt_element_setup(28, (void*)isr28);
    idt_element_setup(29, (void*)isr29);
    idt_element_setup(30, (void*)isr30);
    idt_element_setup(31, (void*)isr31);

	pic_remap(IRQ_MASTER_OFFSET, IRQ_SLAVE_OFFSET);

	idt_element_setup(32, (void*)irq0);
    idt_element_setup(33, (void*)irq1);
    idt_element_setup(34, (void*)irq2);
    idt_element_setup(35, (void*)irq3);
    idt_element_setup(36, (void*)irq4);
    idt_element_setup(37, (void*)irq5);
    idt_element_setup(38, (void*)irq6);
    idt_element_setup(39, (void*)irq7);
    idt_element_setup(40, (void*)irq8);
    idt_element_setup(41, (void*)irq9);
    idt_element_setup(42, (void*)irq10);
    idt_element_setup(43, (void*)irq11);
    idt_element_setup(44, (void*)irq12);
    idt_element_setup(45, (void*)irq13);
    idt_element_setup(46, (void*)irq14);
    idt_element_setup(47, (void*)irq15);
    
    for (int i = 48; i < 256; i++) {
        idt_element_setup(i, (void*)irq10);
    }

    init_irq_handlers();
    idt_load();
}

void setup_irq_handler(uint8_t interrupt_no, void (*handler)()) {
    handlers[interrupt_no] = handler;
}

void init_irq_handlers() {
    int i;
    for (i = IRQ_MASTER_OFFSET; i < IRQ_MASTER_OFFSET + 8; i++){
        handlers[i] = irq_empty_handler;
    }
    for (i = IRQ_SLAVE_OFFSET; i < IRQ_SLAVE_OFFSET + 8; i++){
        handlers[i] = irq_empty_handler;
    }
}

void idt_load() {
	idt_register.base = (void*)&idt;
	idt_register.limit = (uint16_t)(IDT_ENTRIES * sizeof(struct IDT_Entry) - 1);
	__asm__("lidt (%0)" : : "r"(&idt_register));
}

void idt_element_setup(uint8_t n, void* handler_addr) {
	idt[n].offset_lower = (uint32_t)handler_addr & 0xffff;
	idt[n].segment = INIT_CODE_SEG;
	idt[n].zero = 0;
	idt[n].type = 0x8E;
	idt[n].offset_upper = (uint32_t)handler_addr >> 16;
}