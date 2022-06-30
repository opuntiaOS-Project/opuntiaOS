#ifdef __i386__
#include <platform/x86/pmm/settings.h>
#elif __arm__
#include <platform/arm32/pmm/settings.h>
#elif __aarch64__
#include <platform/arm64/pmm/settings.h>
#endif