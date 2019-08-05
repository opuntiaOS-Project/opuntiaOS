#ifndef __oneOS__INTERRUPTS__IDT_H
#define __oneOS__INTERRUPTS__IDT_H

#include <idt_common_data.h>

struct IDT_Entry {
    u_int16 offset_lower;
    u_int16 segment;
    u_int8  zero;
    u_int8  type;
    u_int16 offset_upper;
} __attribute__((packed)) idt[IDT_ENTRIES];

struct IDT_Register {
    u_int16 limit;
    u_int32 base;
} __attribute__((packed)) idt_register;

void idt_element_setup(u_int8 n, u_int32 handler_addr);
void idt_setup();

void setup_irq_handler(u_int8 interrupt_no, void (*handler)());
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
extern void irq_handler_null();

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

#endif