#if defined(__i386__) || defined(__x86_64__)
#include <platform/x86/tasking/signal_impl.h>
#elif __arm__
#include <platform/arm32/tasking/signal_impl.h>
#elif __aarch64__
#include <platform/arm64/tasking/signal_impl.h>
#endif