extern "C" {

extern int __cxa_atexit(void (*)(void*), void*, void*);
int __aeabi_atexit(void* arg, void (*func)(void*), void* dso_handle)
{
    return __cxa_atexit(func, arg, dso_handle);
}
}