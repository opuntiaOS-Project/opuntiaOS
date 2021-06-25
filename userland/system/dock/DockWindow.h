#pragma once

#include <libg/Size.h>
#include <libui/Window.h>

class DockWindow : public UI::Window {
public:
    DockWindow()
        : UI::Window(LG::Size(1024, 44), UI::WindowType::Homescreen)
    {
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
};