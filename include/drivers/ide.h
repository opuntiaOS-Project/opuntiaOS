#ifndef __oneOS__DRIVERS__IDE_H
#define __oneOS__DRIVERS__IDE_H

#include <types.h>
#include <drivers/ata.h>
#include <x86/idt.h>

void ide_install();
void ide_find_devices(device_t *t_device);

#endif // __oneOS__DRIVERS__IDE_H
