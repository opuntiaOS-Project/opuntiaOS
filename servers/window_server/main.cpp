#include "screen.h"
#include <malloc.h>
#include <syscalls.h>

int main()
{
    new Screen();
    Screen::the().run();
    return 0;
}