#ifdef __i386__
#include <drivers/x86/pit.h>
#elif __arm__
#include <drivers/aarch32/sp804.h>
#elif __aarch64__
#include <drivers/aarch64/timer.h>
#endif