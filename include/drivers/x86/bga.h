#ifndef __oneOS__DRIVERS__BGA_H
#define __oneOS__DRIVERS__BGA_H

#include <types.h>
#include <platform/x86/port.h>
#include <drivers/x86/display.h>
#include <mem/kmalloc.h>
#include <drivers/driver_manager.h>

void bga_install();
void bga_init();
void bga_set_resolution(uint16_t width, uint16_t height);

#endif //__oneOS__DRIVERS__BGA_H
