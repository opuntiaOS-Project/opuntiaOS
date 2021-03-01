int errno;

extern int _stdio_init();
extern int _stdio_deinit();

void _libc_init()
{
    _stdio_init();
}

void _libc_deinit()
{
    _stdio_deinit();
}
