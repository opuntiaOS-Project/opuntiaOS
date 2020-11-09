#include "TerminalView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    new UI::App();
    auto* window_ptr = new UI::Window(300, 200);
    auto* terminal_view = new TerminalView(window_ptr->bounds());
    window_ptr->set_superview(terminal_view);
    window_ptr->set_focused_view(terminal_view);
    return UI::App::the().run();
}