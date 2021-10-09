int errno;
char** environ;
int __environ_malloced = 0;

extern int _stdio_init();
extern int _stdio_deinit();
extern int _malloc_init();

void _libc_init(int argc, char* argv[], char* envp[])
{
    environ = envp;
    _malloc_init();
    _stdio_init();
    extern void (*__init_array_start[])(int, char**, char**) __attribute__((visibility("hidden")));
    extern void (*__init_array_end[])(int, char**, char**) __attribute__((visibility("hidden")));

    const unsigned int size = __init_array_end - __init_array_start;
    for (unsigned int i = 0; i < size; i++) {
        (*__init_array_start[i])(argc, argv, envp);
    }
}

void _libc_deinit()
{
    _stdio_deinit();
}
