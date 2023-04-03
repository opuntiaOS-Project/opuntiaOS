#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/vmm/mmu.h>
#elif __arm__
#include <platform/arm32/vmm/mmu.h>
#elif __aarch64__
#include <platform/arm64/vmm/mmu.h>
#elif defined(__riscv) && (__riscv_xlen == 64)
#include <platform/riscv64/vmm/mmu.h>
#endif