#include "port.h"

unsigned char port_byte_in(unsigned short port) {
    unsigned char result_data;
    __asm__("inb %%dx, %%al" : "=a" (result_data) : "d" (port));
    return result_data;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

unsigned short port_word_in(unsigned short port) {
    unsigned short result_data;
    __asm__("inw %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_word_out(unsigned short port, unsigned short data) {
    __asm__("outw %%ax, %%dx" : : "a"(data), "d"(port));
}

unsigned int port_dword_in(unsigned short port) {
    unsigned int result_data;
    __asm__("inl %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_dword_out(unsigned short port, unsigned int data) {
    __asm__("outl %%ax, %%dx" : : "a"(data), "d"(port));
}

unsigned char port_8bit_in(unsigned short port) {
    unsigned char result_data;
    __asm__("inb %%dx, %%al" : "=a" (result_data) : "d" (port));
    return result_data;
}

void port_8bit_out(unsigned short port, unsigned char data) {
    __asm__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

unsigned short port_16bit_in(unsigned short port) {
    unsigned short result_data;
    __asm__("inw %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_16bit_out(unsigned short port, unsigned short data) {
    __asm__("outw %%ax, %%dx" : : "a"(data), "d"(port));
}

unsigned int port_32bit_in(unsigned short port) {
    unsigned int result_data;
    __asm__("inl %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_32bit_out(unsigned short port, unsigned int data) {
    __asm__("outl %%ax, %%dx" : : "a"(data), "d"(port));
}

void io_wait() {
    __asm__("out %%al, $0x80" : : "a"(0)); // writing to "unused" port
}