#include <drivers/timer.h>
#include <tasking/sched.h>

static int ticks_to_sched = 0;

void init_timer() { // Set uo to IRQ0
    ticks_to_sched = SCHED_INT;
    active_proc = 0;
    set_frequency(100);
    set_irq_handler(IRQ0, timer_handler);
}

void set_frequency(uint16_t freq) {
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
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
