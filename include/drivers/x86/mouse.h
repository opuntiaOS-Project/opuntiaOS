#ifndef __oneOS__DRIVERS__M_H
#define __oneOS__DRIVERS__M_H

#include <types.h>
#include <drivers/x86/ata.h>
#include <platform/x86/idt.h>

/* The mouse packet should be aligned to 4 bytes */
struct mouse_packet {
    int16_t x_offset;
    int16_t y_offset;
    uint32_t button_states;
};
typedef struct mouse_packet mouse_packet_t;

bool mouse_install();

#endif // __oneOS__DRIVERS__M_H
