#include <cstddef>
#include <sys/types.h>

extern "C" {
extern void _libc_init();
extern void _libc_deinit();

void _init()
{
    _libc_init();

    extern void (*__init_array_start[])(int, char**, char**) __attribute__((visibility("hidden")));
    extern void (*__init_array_end[])(int, char**, char**) __attribute__((visibility("hidden")));

    const size_t size = __init_array_end - __init_array_start;
    for (size_t i = 0; i < size; i++) {
        (*__init_array_start[i])(0, 0, 0);
    }
}

void _deinit()
{
    _libc_deinit();
}
}