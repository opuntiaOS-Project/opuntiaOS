#ifdef __i386__
#include <platform/x86/tasking/signal_impl.h>
#elif __arm__
#include <platform/arm32/tasking/signal_impl.h>
#elif __aarch64__
#include <platform/aarch64/tasking/signal_impl.h>
#endif