int errno;

extern int _stdio_init();
extern int _stdio_deinit();
extern int _malloc_init();

void _libc_init()
{
    _malloc_init();
    _stdio_init();
}

void _libc_deinit()
{
    _stdio_deinit();
}
