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
}

void _deinit()
{
    __cxa_finalize(nullptr);
    _Z14_libcpp_deinitv();
    _libc_deinit();
}
}