#include "../drivers/port/port.h"

void irq0_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq1_handler() {
	port_byte_out(0x20, 0x20); //EOI
}
 
void irq2_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq3_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq4_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq5_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq6_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq7_handler() {
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq8_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI          
}
 
void irq9_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq10_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq11_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq12_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq13_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq14_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}
 
void irq15_handler() {
    port_byte_out(0xA0, 0x20);
    port_byte_out(0x20, 0x20); //EOI
}