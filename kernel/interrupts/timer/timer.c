#include "timer.h"

void init_timer(u_int32 freq) { // Set uo to IRQ0
    u_int32 divisor = 1193180 / freq;
    u_int8 low  = (u_int8)(divisor & 0xFF);
    u_int8 high = (u_int8)((divisor >> 8) & 0xFF);
    port_byte_out(0x43, 0x36); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}