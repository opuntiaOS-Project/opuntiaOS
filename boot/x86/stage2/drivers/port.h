#ifndef _BOOT_X86_STAGE2_DRIVERS_PORT_H
#define _BOOT_X86_STAGE2_DRIVERS_PORT_H

unsigned char port_read8(unsigned short port);
void port_write8(unsigned short port, unsigned char data);

unsigned short port_read16(unsigned short port);
void port_write16(unsigned short port, unsigned short data);

unsigned int port_read32(unsigned short port);
void port_write32(unsigned short port, unsigned int data);

void port_wait_io();

#endif // _BOOT_X86_STAGE2_DRIVERS_PORT_H