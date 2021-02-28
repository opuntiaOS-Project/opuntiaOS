#pragma once

#include <libui/Window.h>

class DockWindow : public UI::Window {
public:
    DockWindow()
        : UI::Window(1024, 36, UI::WindowType::Dock)
    {
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

};