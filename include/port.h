#ifndef __oneOS__HARDWARECOMMUNICATION__PORT_H
#define __oneOS__HARDWARECOMMUNICATION__PORT_H

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
unsigned short port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);
unsigned int port_dword_in(unsigned short port);
void port_dword_out(unsigned short port, unsigned int data);
void io_wait();

#endif