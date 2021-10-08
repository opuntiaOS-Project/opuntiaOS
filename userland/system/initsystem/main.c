#include <unistd.h>

int main(int argc, char** argv)
{
#ifdef BENCHMARK
    execve("/bin/bench", 0, 0);
    return 0;
#endif
    execve("/System/window_server", 0, 0);
    return 0;
}