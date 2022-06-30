#ifdef __i386__
#include <platform/x86/tasking/dump_impl.h>
#elif __arm__
#include <platform/arm32/tasking/dump_impl.h>
#elif __aarch64__
#include <platform/aarch64/tasking/dump_impl.h>
#endif