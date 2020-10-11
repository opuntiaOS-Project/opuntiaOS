#include <libui/App.h>
#include <libui/Window.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    fork();
    new UI::App();
    auto* window_ptr = new UI::Window();
    window_ptr->receive_event(nullptr);
    window_ptr->run();
    return UI::App::the().run();
}