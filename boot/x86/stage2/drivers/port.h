#ifndef _BOOT_X86_STAGE2_DRIVERS_PORT_H
#define _BOOT_X86_STAGE2_DRIVERS_PORT_H

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
unsigned short port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);
unsigned int port_dword_in(unsigned short port);
void port_dword_out(unsigned short port, unsigned int data);
unsigned char port_8bit_in(unsigned short port);
void port_8bit_out(unsigned short port, unsigned char data);
unsigned short port_16bit_in(unsigned short port);
void port_16bit_out(unsigned short port, unsigned short data);
unsigned int port_32bit_in(unsigned short port);
void port_32bit_out(unsigned short port, unsigned int data);
void io_wait();

#endif // _BOOT_X86_STAGE2_DRIVERS_PORT_H