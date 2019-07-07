#include "irq_handler.h"
#include "../../drivers/display/display.h"


void irq0_handler() {
    print_char('1', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq1_handler() {
    print_char('2', WHITE_ON_BLACK, -1, -1);
	port_byte_out(0x20, 0x20); //EOI
}
 
void irq2_handler() {
    print_char('3', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq3_handler() {
    print_char('4', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq4_handler() {
    print_char('5', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq5_handler() {
    print_char('6', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq6_handler() {
    print_char('7', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq7_handler() {
    print_char('8', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq8_handler() {
    print_char('9', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI          
}
 
void irq9_handler() {
    print_char('a', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq10_handler() {
    print_char('b', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq11_handler() {
    print_char('c', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq12_handler() {
    print_char('d', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq13_handler() {
    print_char('e', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq14_handler() {
    print_char('f', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq15_handler() {
    print_char('z', WHITE_ON_BLACK, -1, -1);
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}