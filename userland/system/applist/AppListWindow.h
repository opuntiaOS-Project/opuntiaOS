#pragma once

#include <libg/Size.h>
#include <libui/Screen.h>
#include <libui/Window.h>

class AppListWindow : public UI::Window {
public:
    AppListWindow()
        : UI::Window("AppList", LG::Size(320, 400), UI::WindowType::AppList)
    {
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
};