#include <isr_handler.h>
#include <display.h>

#define ERR_COLOR RED_ON_BLACK

void isr0_handler() {
    print_string("Division By Zero", ERR_COLOR, -1, -1);
}
 
void isr1_handler() {
    print_string("Debug", ERR_COLOR, -1, -1);
}
 
void isr2_handler() {
    print_string("Non Maskable Interrupt", ERR_COLOR, -1, -1);
}
 
void isr3_handler() {
    print_string("Breakpoint", ERR_COLOR, -1, -1);
}
 
void isr4_handler() {
    print_string("Into Detected Overflow", ERR_COLOR, -1, -1);
}
 
void isr5_handler() {
    print_string("Out of Bounds", ERR_COLOR, -1, -1);
}
 
void isr6_handler() {
    print_string("Invalid Opcode", ERR_COLOR, -1, -1);
}
 
void isr7_handler() {
    print_string("No Coprocessor", ERR_COLOR, -1, -1);
}
 
void isr8_handler() {
    print_string("Double Fault", ERR_COLOR, -1, -1);          
}
 
void isr9_handler() {
    print_string("Coprocessor Segment Overrun", ERR_COLOR, -1, -1);
}
 
void isr10_handler() {
    print_string("Bad TSS", ERR_COLOR, -1, -1);
}
 
void isr11_handler() {
    print_string("Segment Not Present", ERR_COLOR, -1, -1);
}
 
void isr12_handler() {
    print_string("Stack Fault", ERR_COLOR, -1, -1);
}
 
void isr13_handler() {
    print_string("General Protection Fault", ERR_COLOR, -1, -1);
}
 
void isr14_handler() {
    print_string("Page Fault", ERR_COLOR, -1, -1);
}
 
void isr15_handler() {
    print_string("Unknown Interrupt", ERR_COLOR, -1, -1);
}

void isr16_handler() {
    print_string("Coprocessor Fault", ERR_COLOR, -1, -1);
}
 
void isr17_handler() {
    print_string("Alignment Check", ERR_COLOR, -1, -1);
}
 
void isr18_handler() {
    print_string("Machine Check", ERR_COLOR, -1, -1);
}
 
void isr19_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr20_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr21_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr22_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr23_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr24_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);          
}
 
void isr25_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr26_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr27_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr28_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr29_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr30_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}
 
void isr31_handler() {
    print_string("Reserved", ERR_COLOR, -1, -1);
}