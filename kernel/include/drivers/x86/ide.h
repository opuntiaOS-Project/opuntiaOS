#ifndef _KERNEL_DRIVERS_X86_IDE_H
#define _KERNEL_DRIVERS_X86_IDE_H

#include <drivers/x86/ata.h>
#include <libkern/types.h>
#include <platform/x86/idt.h>

void ide_install();
void ide_find_devices(device_t *t_device);

#endif // _KERNEL_DRIVERS_X86_IDE_H
