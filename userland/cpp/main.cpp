#include <libgui/WindowConnection.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    auto window = Window::Window();
    window.run();
    while (1) {}
    return 0;
}