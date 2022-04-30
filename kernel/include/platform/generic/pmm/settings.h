#ifdef __i386__
#include <platform/x86/pmm/settings.h>
#elif __arm__
#include <platform/aarch32/pmm/settings.h>
#elif __aarch64__
#include <platform/aarch64/pmm/settings.h>
#endif