#ifdef __i386__
#include <platform/x86/fpu/fpu.h>
#elif __arm__
#include <platform/aarch32/fpu/fpuv4.h>
#elif __aarch64__
#include <platform/aarch64/fpu/fpu.h>
#endif