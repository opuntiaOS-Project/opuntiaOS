#include <drivers/timer.h>
#include <tasking/sched.h>

static int ticks_to_sched = 0;

void init_timer() { // Set uo to IRQ0
    ticks_to_sched = SCHED_INT;
    set_frequency(100);
    set_irq_handler(IRQ0, timer_handler);
}

void set_frequency(u_int16 freq) {
    u_int32 divisor = 1193180 / freq;
    u_int8 low  = (u_int8)(divisor & 0xFF);
    u_int8 high = (u_int8)((divisor >> 8) & 0xFF);
    port_byte_out(0x43, 0x36); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

void timer_handler() {
    // ticks++;
    if (active_proc) {
        ticks_to_sched--;
        if (ticks_to_sched < 0) {
            ticks_to_sched = SCHED_INT;
            presched();
        }
    }
}
