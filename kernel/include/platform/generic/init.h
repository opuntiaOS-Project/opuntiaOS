#ifdef __i386__
#include <platform/x86/init.h>
#elif __arm__
#include <platform/arm32/init.h>
#elif __aarch64__
#include <platform/arm64/init.h>
#endif