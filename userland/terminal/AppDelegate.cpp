#include "TerminalView.h"
#include "TerminalViewController.h"
#include <libui/AppDelegate.h>

int setup_shell()
{
    int ptmx = posix_openpt(O_RDONLY);
    int f = fork();
    if (f == 0) {
        char* pname = ptsname(ptmx);
        if (!pname) {
            return -1;
        }
        Logger::debug << "open " << pname << std::endl;
        close(0);
        close(1);
        open(pname, O_RDONLY);
        open(pname, O_WRONLY);
        execve("/bin/onesh", 0, 0);
    }
    return ptmx;
}

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_window_size() const override { return LG::Size(400, 300); }
    const char* icon_path() const override { return "/res/icons/apps/terminal.icon"; }

    bool application() override
    {
        int ptmx = setup_shell();
        auto& window = std::oneos::construct<UI::Window>(window_size(), icon_path());
        auto& superview = window.create_superview<TerminalView, TerminalViewController>(ptmx);
        window.set_focused_view(superview);
        window.set_frame_style(LG::Color(0x181818));
        window.set_title("Terminal");
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
