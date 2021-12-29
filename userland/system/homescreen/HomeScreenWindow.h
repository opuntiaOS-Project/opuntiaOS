#pragma once

#include "HomeScreenView.h"
#include <libg/Size.h>
#include <libui/Window.h>

class HomeScreenWindow : public UI::Window {
public:
    HomeScreenWindow(const LG::Size& size)
        : UI::Window("Homescreen", size, UI::WindowType::Homescreen)
    {
        if (fork() == 0) {
            execve("/System/applist", 0, 0);
            std::abort();
        }
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override
    {
        if (event->type() == UI::Event::Type::NotifyWindowCreateEvent) {
            UI::NotifyWindowCreateEvent& own_event = *(UI::NotifyWindowCreateEvent*)event.get();
            HomeScreenView* it = (HomeScreenView*)superview();
            it->on_window_create(own_event.bundle_id(), own_event.icon_path(), own_event.window_id(), own_event.window_type());
        }

        Window::receive_event(std::move(event));
    }
};