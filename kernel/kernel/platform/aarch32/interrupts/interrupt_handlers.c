#include <drivers/aarch32/gicv2.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/system.h>
#include <platform/aarch32/tasking/trapframe.h>
#include <platform/generic/registers.h>
#include <syscalls/handlers.h>

/* IRQ */
static irq_handler_t _irq_handlers[IRQ_HANDLERS_MAX];
static void _irq_empty_handler();
static inline void _irq_redirect(int int_no);
static void init_irq_handlers();

static inline uint32_t is_interrupt_enabled()
{
    return ((read_cpsr() >> 7) & 1) == 0;
}

static inline void enable_interrupts()
{
    asm volatile("cpsie i");
}

static inline void disable_interrupts()
{
    asm volatile("cpsid i");
}

static void dump_tf(trapframe_t* tf)
{
    for (int i = 0; i < 13; i++) {
        log("r[%d]: %x", i, tf->r[i]);
    }
    log("sp: %x", tf->user_sp);
    log("ip: %x", tf->user_ip);
    log("fl: %x", tf->user_flags);
}

void interrupts_setup()
{
    disable_interrupts();
    set_abort_stack((uint32_t)&STACK_ABORT_TOP);
    set_svc_stack((uint32_t)&STACK_SVC_TOP);
    set_irq_stack((uint32_t)&STACK_IRQ_TOP);
    init_irq_handlers();
}

void gic_setup()
{
    gicv2_install();
}

void reset_handler()
{
    log("reset_handler");
}

void undefined_handler()
{
    uint32_t val;
    asm volatile("mov %0, lr"
                 : "=r"(val)
                 :);
    log("undefined_handler address : %x", val);
    while (1) { }
}

void svc_handler(trapframe_t* tf)
{
    sys_handler(tf);
}

void prefetch_abort_handler()
{
    uint32_t val;
    asm volatile("mov %0, lr"
                 : "=r"(val)
                 :);
    log("prefetch_abort_handler address : %x", val);
    while (1) { }
}

void data_abort_handler(trapframe_t* tf)
{
    system_disable_interrupts();
    uint32_t fault_addr = read_far();
    uint32_t info = read_dfsr();
    uint32_t is_pl0 = read_spsr() & 0xf; // See CPSR M field values
    info |= ((is_pl0 != 0) << 31); // Set the 31bit as type
    // log("data_abort_handler: %x %x %x", fault_addr, info, tf->user_ip);
    vmm_page_fault_handler(info, fault_addr);
    system_enable_interrupts_only_counter();
}

/**
 * IRQ
 */

static void _irq_empty_handler()
{
    return;
}

static void init_irq_handlers()
{
    for (int i = 0; i < IRQ_HANDLERS_MAX; i++) {
        _irq_handlers[i] = _irq_empty_handler;
    }
}

static inline void _irq_redirect(irq_line_t line)
{
    _irq_handlers[line]();
}

void irq_handler(trapframe_t* tf)
{
    system_disable_interrupts();
    /* Remove gicv2 call from here */
    uint32_t int_disc = gicv2_interrupt_descriptor();
    /* We end the interrupt before handle it, since we can
       call sched() and not return here. */
    gicv2_end(int_disc);
    _irq_redirect(int_disc & 0x1ff);
    system_enable_interrupts_only_counter();
}

void fast_irq_handler()
{
    log("fast_irq_handler");
}

void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_type_t type, irq_handler_t func)
{
    _irq_handlers[line] = func;
    gicv2_enable_irq(line, prior, type);
}