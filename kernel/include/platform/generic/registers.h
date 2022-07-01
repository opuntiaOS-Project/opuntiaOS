#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/registers.h>
#elif __arm__
#include <platform/arm32/registers.h>
#elif __aarch64__
#include <platform/arm64/registers.h>
#endif