#include "DockWindow.h"
#include "DockView.h"

void DockWindow::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == UI::Event::Type::NotifyWindowCreateEvent) {
        UI::NotifyWindowCreateEvent& own_event = *(UI::NotifyWindowCreateEvent*)event.get();
        DockView* it = (DockView*)superview();
        it->on_window_create(own_event.bundle_id(), own_event.icon_path(), own_event.window_id());
    }
    if (event->type() == UI::Event::Type::NotifyWindowStatusChangedEvent) {
        UI::NotifyWindowStatusChangedEvent& own_event = *(UI::NotifyWindowStatusChangedEvent*)event.get();
        DockView* it = (DockView*)superview();
        if (own_event.type() == UI::WindowStatusUpdateType::Removed) {
            it->on_window_remove(own_event.changed_window_id());
        }
        if (own_event.type() == UI::WindowStatusUpdateType::Minimized) {
            it->on_window_minimize(own_event.changed_window_id());
        }
    }
    if (event->type() == UI::Event::Type::NotifyWindowIconChangedEvent) {
        UI::NotifyWindowIconChangedEvent& own_event = *(UI::NotifyWindowIconChangedEvent*)event.get();
        DockView* it = (DockView*)superview();
        it->set_icon(own_event.changed_window_id(), own_event.icon_path());
    }
    if (event->type() == UI::Event::Type::NotifyWindowTitleChangedEvent) {
        UI::NotifyWindowTitleChangedEvent& own_event = *(UI::NotifyWindowTitleChangedEvent*)event.get();
        DockView* it = (DockView*)superview();
        it->set_title(own_event.changed_window_id(), own_event.title());
    }

    Window::receive_event(std::move(event));
}