#ifndef _KERNEL_DRIVERS_X86_MOUSE_H
#define _KERNEL_DRIVERS_X86_MOUSE_H

#include <drivers/generic/mouse.h>
#include <drivers/x86/ata.h>
#include <platform/x86/idt.h>
#include <libkern/types.h>

bool mouse_install();

#endif // _KERNEL_DRIVERS_X86_MOUSE_H
