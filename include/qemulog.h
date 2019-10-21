#ifndef __oneOS__UTILS__QEMULOG_H
#define __oneOS__UTILS__QEMULOG_H

#include <types.h>
#include <x86/port.h>

#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

void log(const char* message);

#endif //__oneOS__UTILS__QEMULOG_H