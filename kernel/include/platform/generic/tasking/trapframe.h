#ifdef __i386__
#include <platform/x86/tasking/trapframe.h>
#elif __arm__
#include <platform/arm32/tasking/trapframe.h>
#elif __aarch64__
#include <platform/aarch64/tasking/trapframe.h>
#endif