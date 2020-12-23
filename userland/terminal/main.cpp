#include "TerminalView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <std/Dbg.h>

int setup_shell()
{
    int ptmx = open("/dev/ptmx", O_RDONLY);
    int f = fork();
    if (f == 0) {
        close(0);
        close(1);
        open("/dev/pts1", O_RDONLY);
        open("/dev/pts1", O_WRONLY);
        execve("/bin/onesh", 0, 0);
    }
    return ptmx;
}

int main(int argc, char** argv)
{
    int ptmx = setup_shell();
    new UI::App();
    auto* window_ptr = new UI::Window(400, 300);
    auto* terminal_view = new TerminalView(window_ptr->bounds(), ptmx);
    window_ptr->set_superview(terminal_view);
    window_ptr->set_focused_view(terminal_view);
    window_ptr->set_title("Terminal");
    return UI::App::the().run();
}