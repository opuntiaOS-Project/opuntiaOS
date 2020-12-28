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
    auto& app = create<UI::App>();
    auto& window = create<UI::Window>(400, 300, "/res/icons/apps/terminal.icon");
    auto& terminal_view = window.create_superview<TerminalView>(ptmx);
    window.set_focused_view(terminal_view);
    window.set_frame_style(LG::Color(0x181818));
    window.set_title("Terminal");
    return app.run();
}