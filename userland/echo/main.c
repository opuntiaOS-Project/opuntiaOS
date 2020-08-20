#include <libsystem/syscalls.h>

int main(int argc, char** argv)
{
    write(1, (char*)argv[1], 6);
    return 0;
}