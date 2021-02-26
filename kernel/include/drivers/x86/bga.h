#ifndef _KERNEL_DRIVERS_X86_BGA_H
#define _KERNEL_DRIVERS_X86_BGA_H

#include <libkern/types.h>
#include <platform/x86/port.h>
#include <drivers/x86/display.h>
#include <mem/kmalloc.h>
#include <drivers/driver_manager.h>

void bga_install();
void bga_init();
void bga_set_resolution(uint16_t width, uint16_t height);

#endif //_KERNEL_DRIVERS_X86_BGA_H
