#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/system.h>
#elif __arm__
#include <platform/arm32/system.h>
#elif __aarch64__
#include <platform/arm64/system.h>
#elif defined(__riscv) && (__riscv_xlen == 64)
#include <platform/riscv64/system.h>
#endif