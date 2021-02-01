#ifndef __oneOS__AARCH32__INTERRUPTS_H
#define __oneOS__AARCH32__INTERRUPTS_H

#include <types.h>
#include <utils/mask.h>

#define IRQ_HANDLERS_MAX 256

typedef int irq_type_t;
typedef int irq_line_t;
typedef uint8_t irq_priority_t;
typedef void (*irq_handler_t)();

enum IRQTypeMasks {
    MASKDEFINE(IRQ_TYPE_EDGE_TRIGGERED, 0, 1),
};

void interrupts_setup();

extern char STACK_ABORT_TOP;
extern char STACK_IRQ_TOP;
extern char STACK_SVC_TOP;
extern char STACK_TOP;

extern void swi(uint32_t num);
extern void set_svc_stack(uint32_t stack);
extern void set_irq_stack(uint32_t stack);
extern void set_abort_stack(uint32_t stack);

extern void reset_handler();
extern void undefined_handler();
extern void svc_handler();
extern void prefetch_abort_handler();
extern void data_abort_handler();
extern void irq_handler();
extern void fast_irq_handler();

void reg_int(int no);
void irq_register_handler(irq_line_t line, irq_priority_t prior, irq_type_t type, irq_handler_t func);

void gic_setup();

#endif /* __oneOS__AARCH32__INTERRUPTS_H */