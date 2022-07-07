#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int ret = 1;

void save_on_terminate()
{
    ret = 0;
}

int main(int argc, char** argv)
{
    sigaction(SIGTERM, save_on_terminate);
    raise(SIGTERM);
    return ret;
}