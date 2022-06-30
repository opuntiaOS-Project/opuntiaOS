#ifdef __i386__
#include <platform/x86/system.h>
#elif __arm__
#include <platform/arm32/system.h>
#elif __aarch64__
#include <platform/aarch64/system.h>
#endif