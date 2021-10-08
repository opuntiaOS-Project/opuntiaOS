#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void save_on_terminate()
{
    printf("Saving file and exiting");
    fflush(stdout);
}

int main(int argc, char** argv)
{
    sigaction(SIGTERM, save_on_terminate);
    while (1) {
        // infinite loop
    }
    return 0;
}