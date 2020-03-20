#include <x86/idt.h>
#define USER true
#define SYS false

void lidt(void* p, uint16_t size) {
    volatile uint16_t pd[3];
    pd[0] = size-1;
    pd[1] = (uint32_t)p;
    pd[2] = (uint32_t)p >> 16;
	asm volatile("lidt (%0)" : : "r"(pd));
}

void idt_setup() {
	idt_element_setup(0, (void*)isr0, SYS);
	idt_element_setup(1, (void*)isr1, SYS);
	idt_element_setup(2, (void*)isr2, SYS);
	idt_element_setup(3, (void*)isr3, SYS);
	idt_element_setup(4, (void*)isr4, SYS);
	idt_element_setup(5, (void*)isr5, SYS);
	idt_element_setup(6, (void*)isr6, SYS);
	idt_element_setup(7, (void*)isr7, SYS);
	idt_element_setup(8, (void*)isr8, SYS);
	idt_element_setup(9, (void*)isr9, SYS);
	idt_element_setup(10, (void*)isr10, SYS);
    idt_element_setup(11, (void*)isr11, SYS);
    idt_element_setup(12, (void*)isr12, SYS);
    idt_element_setup(13, (void*)isr13, SYS);
    idt_element_setup(14, (void*)isr14, SYS);
    idt_element_setup(15, (void*)isr15, SYS);
    idt_element_setup(16, (void*)isr16, SYS);
    idt_element_setup(17, (void*)isr17, SYS);
    idt_element_setup(18, (void*)isr18, SYS);
    idt_element_setup(19, (void*)isr19, SYS);
    idt_element_setup(20, (void*)isr20, SYS);
    idt_element_setup(21, (void*)isr21, SYS);
    idt_element_setup(22, (void*)isr22, SYS);
    idt_element_setup(23, (void*)isr23, SYS);
    idt_element_setup(24, (void*)isr24, SYS);
    idt_element_setup(25, (void*)isr25, SYS);
    idt_element_setup(26, (void*)isr26, SYS);
    idt_element_setup(27, (void*)isr27, SYS);
    idt_element_setup(28, (void*)isr28, SYS);
    idt_element_setup(29, (void*)isr29, SYS);
    idt_element_setup(30, (void*)isr30, SYS);
    idt_element_setup(31, (void*)isr31, SYS);

	pic_remap(IRQ_MASTER_OFFSET, IRQ_SLAVE_OFFSET);

	idt_element_setup(32, (void*)irq0, SYS);
    idt_element_setup(33, (void*)irq1, SYS);
    idt_element_setup(34, (void*)irq2, SYS);
    idt_element_setup(35, (void*)irq3, SYS);
    idt_element_setup(36, (void*)irq4, SYS);
    idt_element_setup(37, (void*)irq5, SYS);
    idt_element_setup(38, (void*)irq6, SYS);
    idt_element_setup(39, (void*)irq7, SYS);
    idt_element_setup(40, (void*)irq8, SYS);
    idt_element_setup(41, (void*)irq9, SYS);
    idt_element_setup(42, (void*)irq10, SYS);
    idt_element_setup(43, (void*)irq11, SYS);
    idt_element_setup(44, (void*)irq12, SYS);
    idt_element_setup(45, (void*)irq13, SYS);
    idt_element_setup(46, (void*)irq14, SYS);
    idt_element_setup(47, (void*)irq15, SYS);
    

    idt_element_setup(48, (void*)syscall1, USER);
    idt_element_setup(49, (void*)syscall2, USER);
    idt_element_setup(50, (void*)syscall3, USER);
    idt_element_setup(51, (void*)syscall4, USER);
    for (int i = 52; i < 256; i++) {
        idt_element_setup(i, (void*)syscall1, USER);
    }

    init_irq_handlers();
    lidt(idt, sizeof(idt));
    asm volatile("sti");
}

void set_irq_handler(uint8_t interrupt_no, void (*handler)()) {
    handlers[interrupt_no] = (void*)handler;
}

inline void init_irq_handlers() {
    int i;
    for (i = IRQ_MASTER_OFFSET; i < IRQ_MASTER_OFFSET + 8; i++){
        handlers[i] = (void*)irq_empty_handler;
    }
    for (i = IRQ_SLAVE_OFFSET; i < IRQ_SLAVE_OFFSET + 8; i++){
        handlers[i] = (void*)irq_empty_handler;
    }
}

inline void idt_element_setup(uint8_t n, void* handler_addr, bool is_user) {
	idt[n].offset_lower = (uint32_t)handler_addr & 0xffff;
	idt[n].segment = INIT_CODE_SEG;
	idt[n].zero = 0;
    idt[n].type = 0x8E;
    // setting user type
    // now user can call this sw interrupts (syscalls)
    if (is_user) {
	    idt[n].type |= (0b1100000);
    }
	idt[n].offset_upper = (uint32_t)handler_addr >> 16;
}   