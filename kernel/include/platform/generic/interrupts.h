#ifdef __i386__
#include <platform/x86/irq.h>
#elif __arm__
#include <platform/aarch32/interrupts.h>
#elif __aarch64__
#include <platform/aarch64/interrupts.h>
#endif