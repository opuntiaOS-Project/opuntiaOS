extern void _libc_init();
extern void _libc_deinit();

void _init()
{
    _libc_init();
}

void _deinit()
{
    _libc_deinit();
}