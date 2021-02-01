#ifdef __i386__
#include <drivers/x86/uart.h>
#elif __arm__
#include <drivers/aarch32/uart.h>
#endif