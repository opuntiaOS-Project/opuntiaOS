#include "DockWindow.h"
#include "DockView.h"
#include <std/Dbg.h>

void DockWindow::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == UI::Event::Type::NotifyWindowStatusChangedEvent) {
        UI::NotifyWindowStatusChangedEvent& own_event = *(UI::NotifyWindowStatusChangedEvent*)event.get();
        DockView* it = (DockView*)superview();
        if (own_event.type() == 0) {
            it->new_entity(own_event.changed_window_id());    
        }
        if (own_event.type() == 2) {
            it->remove_entity(own_event.changed_window_id());
        }
    }
    if (event->type() == UI::Event::Type::NotifyWindowIconChangedEvent) {
        UI::NotifyWindowIconChangedEvent& own_event = *(UI::NotifyWindowIconChangedEvent*)event.get();
        DockView* it = (DockView*)superview();
        it->set_icon(own_event.changed_window_id(), own_event.icon_path());
    }
    Window::receive_event(move(event));
}