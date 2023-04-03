#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/syscalls/params.h>
#elif __arm__
#include <platform/arm32/syscalls/params.h>
#elif __aarch64__
#include <platform/arm64/syscalls/params.h>
#elif defined(__riscv) && (__riscv_xlen == 64)
#include <platform/riscv64/syscalls/params.h>
#endif