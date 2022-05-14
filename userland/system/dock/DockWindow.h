#pragma once

#include <libg/Size.h>
#include <libui/Screen.h>
#include <libui/Window.h>

class DockWindow : public UI::Window {
public:
    DockWindow()
        : UI::Window("Dock", LG::Size(UI::Screen::main().bounds().width(), 46), UI::WindowType::Homescreen)
    {
        if (fork() == 0) {
            for (int i = 3; i < 32; i++) {
                close(i);
            }
            execlp("/System/applist", "/System/applist", NULL);
            std::abort();
        }

        if (fork() == 0) {
            execlp("/Applications/about.app/Content/about", "/Applications/about.app/Content/about", NULL);
            std::abort();
        }

        if (fork() == 0) {
            execlp("/Applications/activity_monitor.app/Content/activity_monitor", "/Applications/activity_monitor.app/Content/activity_monitor", NULL);
            std::abort();
        }
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
};