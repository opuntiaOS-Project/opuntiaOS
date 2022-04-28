#include "cxxabi.h"

#define __EXIT_ENTRIES_COUNT (64)

extern "C" {
VIS_HIDDEN void* __dso_handle;

struct VIS_HIDDEN __termination_info {
    void (*func)(void*);
    void* arg;
    void* dso_handle;
};

static struct __termination_info __exit_info[__EXIT_ENTRIES_COUNT];
static int __exit_counter = 0;

int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle)
{
    if (__exit_counter >= __EXIT_ENTRIES_COUNT) {
        return -1;
    }

    __exit_info[__exit_counter++] = {
        .func = func,
        .arg = arg,
        .dso_handle = dso_handle,
    };
    return 0;
}

void __cxa_finalize(void* dso_handle)
{
    for (int i = __exit_counter - 1; i >= 0; i--) {
        if (dso_handle && __exit_info[i].dso_handle != dso_handle) {
            continue;
        }
        __exit_info[i].func(__exit_info[i].arg);
    }
}
}