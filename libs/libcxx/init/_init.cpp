#include <cstddef>
#include <sys/types.h>

extern "C" {
extern void _libc_init();
extern void _libc_deinit();
extern void _Z12_libcpp_initv();
extern void _Z14_libcpp_deinitv();
extern void __cxa_finalize(void*);

void _init()
{
    _libc_init();
    _Z12_libcpp_initv();

    extern void (*__init_array_start[])(int, char**, char**) __attribute__((visibility("hidden")));
    extern void (*__init_array_end[])(int, char**, char**) __attribute__((visibility("hidden")));

    const size_t size = __init_array_end - __init_array_start;
    for (size_t i = 0; i < size; i++) {
        (*__init_array_start[i])(0, 0, 0);
    }
}

void _deinit()
{
    __cxa_finalize(nullptr);
    _Z14_libcpp_deinitv();
    _libc_deinit();
}
}