#include <x86/port.h>

uint8_t port_byte_in(uint16_t port) {
    uint8_t result_data;
    __asm__("inb %%dx, %%al" : "=a" (result_data) : "d" (port));
    return result_data;
}

void port_byte_out(uint16_t port, uint8_t data) {
    __asm__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result_data;
    __asm__("inw %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_word_out(uint16_t port, uint16_t data) {
    __asm__("outw %%ax, %%dx" : : "a"(data), "d"(port));
}

uint32_t port_dword_in(uint16_t port) {
    uint32_t result_data;
    __asm__("inl %%dx, %%eax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_dword_out(uint16_t port, uint32_t data) {
    __asm__("outl %%eax, %%dx" : : "a"(data), "d"(port));
}

uint8_t port_8bit_in(uint16_t port) {
    uint8_t result_data;
    __asm__("inb %%dx, %%al" : "=a" (result_data) : "d" (port));
    return result_data;
}

void port_8bit_out(uint16_t port, uint8_t data) {
    __asm__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

uint16_t port_16bit_in(uint16_t port) {
    uint16_t result_data;
    __asm__("inw %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_16bit_out(uint16_t port, uint16_t data) {
    __asm__("outw %%ax, %%dx" : : "a"(data), "d"(port));
}

uint32_t port_32bit_in(uint16_t port) {
    uint32_t result_data;
    __asm__("inl %%dx, %%eax" : "=a"(result_data) : "d"(port));
    return result_data;
}

void port_32bit_out(uint16_t port, uint32_t data) {
    __asm__("outl %%eax, %%dx" : : "a"(data), "d"(port));
}

void io_wait() {
    __asm__("out %%al, $0x80" : : "a"(0)); // writing to "unused" port
}