#include <cxxabi.h>

extern "C" {
void* __dso_handle __attribute__((visibility("hidden")));

int __cxa_atexit(void (*)(void*), void*, void*)
{
    return 0;
}

void __cxa_finalize(void*)
{
}
}