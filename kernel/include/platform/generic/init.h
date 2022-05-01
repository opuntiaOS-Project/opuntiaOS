#ifdef __i386__
#include <platform/x86/init.h>
#elif __arm__
#include <platform/aarch32/init.h>
#elif __aarch64__
#include <platform/aarch64/init.h>
#endif