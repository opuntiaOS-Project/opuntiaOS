#ifdef __i386__
#include <drivers/x86/rtc.h>
#elif __arm__
#include <drivers/aarch32/pl031.h>
#endif