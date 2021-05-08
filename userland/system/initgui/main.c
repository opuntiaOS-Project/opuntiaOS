#include <unistd.h>

int main(int argc, char** argv)
{
    execve("/System/window_server", 0, 0);
    return 0;
}