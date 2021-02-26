#ifndef _KERNEL_PLATFORM_X86_IDT_H
#define _KERNEL_PLATFORM_X86_IDT_H

#include <platform/x86/pic.h>
#include <platform/x86/port.h>
#include <platform/x86/tasking/trapframe.h>
#include <libkern/types.h>

#define INIT_CODE_SEG 0x08
#define INIT_DATA_SEG 0x10
#define IDT_ENTRIES 0x100

#define IRQ_MASTER_OFFSET 32
#define IRQ_SLAVE_OFFSET 40

struct idt_entry { // call gate
    uint16_t offset_lower;
    uint16_t segment;
    uint8_t zero;
    uint8_t type;
    uint16_t offset_upper;
} __attribute__((packed));

extern struct idt_entry idt[IDT_ENTRIES];
extern void** handlers[IDT_ENTRIES];

void idt_element_setup(uint8_t n, void* handler_addr, bool user);
void interrupts_setup();

void set_irq_handler(uint8_t interrupt_no, void (*handler)());
void init_irq_handlers();

/* ISRs reserved for CPU exceptions */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
/* IRQ definitions */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void irq_null();
extern void irq_empty_handler();

extern void syscall();

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#endif // _KERNEL_PLATFORM_X86_IDT_H
