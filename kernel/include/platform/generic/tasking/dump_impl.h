#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/tasking/dump_impl.h>
#elif __arm__
#include <platform/arm32/tasking/dump_impl.h>
#elif __aarch64__
#include <platform/arm64/tasking/dump_impl.h>
#elif defined(__riscv) && (__riscv_xlen == 64)
#include <platform/riscv64/tasking/dump_impl.h>
#endif