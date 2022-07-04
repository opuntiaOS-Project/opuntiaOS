#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/fpu/fpu.h>
#elif __arm__
#include <platform/arm32/fpu/fpuv4.h>
#elif __aarch64__
#include <platform/arm64/fpu/fpu.h>
#endif