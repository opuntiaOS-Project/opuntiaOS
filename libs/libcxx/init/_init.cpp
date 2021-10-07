#include <cstddef>
#include <sys/types.h>

extern "C" {
extern void _libc_init(int argc, char* argv[], char* envp[]);
extern void _libc_deinit();
extern void _Z12_libcpp_initv();
extern void _Z14_libcpp_deinitv();
extern void __cxa_finalize(void*);

void _init(int argc, char* argv[], char* envp[])
{
    _libc_init(argc, argv, envp);
    _Z12_libcpp_initv();
}

void _deinit()
{
    __cxa_finalize(nullptr);
    _Z14_libcpp_deinitv();
    _libc_deinit();
}
}