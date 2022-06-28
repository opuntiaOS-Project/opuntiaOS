#include "port.h"

unsigned char port_read8(unsigned short port)
{
    unsigned char result_data;
    asm volatile("inb %%dx, %%al"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write8(unsigned short port, unsigned char data)
{
    asm volatile("outb %%al, %%dx"
                 :
                 : "a"(data), "d"(port));
}

unsigned short port_read16(unsigned short port)
{
    unsigned short result_data;
    asm volatile("inw %%dx, %%ax"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write16(unsigned short port, unsigned short data)
{
    asm volatile("outw %%ax, %%dx"
                 :
                 : "a"(data), "d"(port));
}

unsigned int port_read32(unsigned short port)
{
    unsigned int result_data;
    asm volatile("inl %%dx, %%eax"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write32(unsigned short port, unsigned int data)
{
    asm volatile("outl %%eax, %%dx"
                 :
                 : "a"(data), "d"(port));
}

void port_wait_io()
{
    asm volatile("out %%al, $0x80"
                 :
                 : "a"(0)); // writing to "unused" port
}