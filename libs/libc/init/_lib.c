int errno;

extern int _stdio_init();
extern int _stdio_deinit();
extern int _malloc_init();

void _libc_init()
{
    _malloc_init();
    _stdio_init();
    extern void (*__init_array_start[])(int, char**, char**) __attribute__((visibility("hidden")));
    extern void (*__init_array_end[])(int, char**, char**) __attribute__((visibility("hidden")));

    const unsigned int size = __init_array_end - __init_array_start;
    for (unsigned int i = 0; i < size; i++) {
        (*__init_array_start[i])(0, 0, 0);
    }
}

void _libc_deinit()
{
    _stdio_deinit();
}
