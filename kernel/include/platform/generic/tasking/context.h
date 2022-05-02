#ifdef __i386__
#include <platform/x86/tasking/context.h>
#elif __arm__
#include <platform/aarch32/tasking/context.h>
#elif __aarch64__
#include <platform/aarch64/tasking/context.h>
#endif