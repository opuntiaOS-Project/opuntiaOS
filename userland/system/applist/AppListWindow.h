#pragma once

#include <libg/Size.h>
#include <libui/Screen.h>
#include <libui/Window.h>

class AppListWindow : public UI::Window {
public:
    AppListWindow(const LG::Size& size)
        : UI::Window("AppList", size, UI::WindowType::AppList)
    {
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
};