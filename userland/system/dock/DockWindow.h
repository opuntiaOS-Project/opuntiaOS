#pragma once

#include <libg/Size.h>
#include <libui/Screen.h>
#include <libui/Window.h>

class DockWindow : public UI::Window {
public:
    DockWindow()
        : UI::Window("Dock", LG::Size(UI::Screen::main().bounds().width(), 50), UI::WindowType::Homescreen)
    {
        if (fork() == 0) {
            for (int i = 3; i < 32; i++) {
                close(i);
            }
            execlp("/System/applist", "/System/applist", NULL);
            std::abort();
        }
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
};