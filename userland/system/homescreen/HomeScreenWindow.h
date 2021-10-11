#pragma once

#include <libg/Size.h>
#include <libui/Window.h>

class HomeScreenWindow : public UI::Window {
public:
    HomeScreenWindow(const LG::Size& size)
        : UI::Window("Homescreen", size, UI::WindowType::Homescreen)
    {
    }
};