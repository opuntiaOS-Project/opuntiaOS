#include <libgui/WindowConnection.h>
#include <string.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    auto res = Window::WindowConnection(socket(PF_LOCAL, 0, 0));
    while (1) {}
    return 0;
}