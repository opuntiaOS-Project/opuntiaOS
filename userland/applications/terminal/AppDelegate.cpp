#include "TerminalView.h"
#include "TerminalViewController.h"
#include <csignal>
#include <libui/AppDelegate.h>

static int shell_pid = 0;

int setup_shell()
{
    int ptmx = posix_openpt(O_RDONLY);
    if (ptmx < 0) {
        std::abort();
    }

    int f = fork();
    if (f == 0) {
        char* pname = ptsname(ptmx);
        if (!pname) {
            return -1;
        }
        close(0);
        close(1);
        close(2);
        open(pname, O_RDONLY);
        open(pname, O_WRONLY);
        open(pname, O_WRONLY);
        execve("/bin/onesh", 0, 0);
    }

    shell_pid = f;
    return ptmx;
}

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(400, 300); }
    const char* icon_path() const override { return "/res/icons/apps/terminal.icon"; }

    bool application() override
    {
        int ptmx = setup_shell();
        auto& window = std::opuntiaos::construct<UI::Window>(window_size(), icon_path());
        window.set_frame_style(LG::Color(58, 58, 64), TextStyle::Light);
        window.set_title("Terminal");

        auto& superview = window.create_superview<TerminalView, TerminalViewController>(ptmx);
        window.set_focused_view(superview);
        return true;
    }

    void application_will_terminate() override
    {
        std::kill(shell_pid, 9);
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
