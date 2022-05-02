#ifdef __i386__
#include <platform/x86/vmm/mmu.h>
#elif __arm__
#include <platform/aarch32/vmm/mmu.h>
#elif __aarch64__
#include <platform/aarch64/vmm/mmu.h>
#endif