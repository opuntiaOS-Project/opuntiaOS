#include "TerminalView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <memory>
#include <std/Dbg.h>
#include <stdlib.h>
#include <unistd.h>

int setup_shell()
{
    int ptmx = posix_openpt(O_RDONLY);
    int f = fork();
    if (f == 0) {
        char* pname = ptsname(ptmx);
        if (!pname) {
            return -1;
        }
        Dbg() << "open " << pname << "\n";
        close(0);
        close(1);
        open(pname, O_RDONLY);
        open(pname, O_WRONLY);
        execve("/bin/onesh", 0, 0);
    }
    return ptmx;
}

int main(int argc, char** argv)
{
    int ptmx = setup_shell();
    auto& app = std::oneos::construct<UI::App>();
    auto& window = std::oneos::construct<UI::Window>(400, 300, "/res/icons/apps/terminal.icon");
    auto& terminal_view = window.create_superview<TerminalView>(ptmx);
    window.set_focused_view(terminal_view);
    window.set_frame_style(LG::Color(0x181818));
    window.set_title("Terminal");
    return app.run();
}