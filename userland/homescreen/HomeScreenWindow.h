#pragma once

#include <libg/Size.h>
#include <libui/Window.h>

class HomeScreenWindow : public UI::Window {
public:
    HomeScreenWindow(const LG::Size& size)
        : UI::Window(size, UI::WindowType::Homescreen)
    {
    }
};