#ifdef __i386__
#include <drivers/x86/fpu.h>
#elif __arm__
#include <drivers/aarch32/fpuv4.h>
#endif