extern void _libc_init(int argc, char* argv[], char* envp[]);
extern void _libc_deinit();

void _init(int argc, char* argv[], char* envp[])
{
    _libc_init(argc, argv, envp);
}

void _deinit()
{
    _libc_deinit();
}