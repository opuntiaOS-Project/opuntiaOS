#ifdef __i386__
#include <platform/x86/syscalls/params.h>
#elif __arm__
#include <platform/aarch32/syscalls/params.h>
#elif __aarch64__
#include <platform/aarch64/syscalls/params.h>
#endif