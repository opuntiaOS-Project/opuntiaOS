/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libui/App.h>
#include <libui/Responder.h>
#include <libui/Window.h>
#include <std/Dbg.h>

namespace UI {

bool Responder::send_invalidate_message_to_server(const LG::Rect& rect) const
{
    auto& app = App::the();
    InvalidateMessage msg(Connection::the().key(), app.window().id(), rect);
    return app.connection().send_async_message(msg);
}

void Responder::send_layout_message(Window& win, UI::View* for_view)
{
    LFoundation::EventLoop::the().add(win, new LayoutEvent(for_view));
}

void Responder::send_display_message_to_self(Window& win, const LG::Rect& display_rect)
{
    if (!m_display_message_sent || m_prev_display_message != display_rect) {
        LFoundation::EventLoop::the().add(win, new DisplayEvent(display_rect));
        m_display_message_sent = true;
        m_prev_display_message = display_rect;
    }
}

void Responder::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::MouseEvent) {
        MouseEvent& own_event = *(MouseEvent*)event.get();
        receive_mouse_move_event(own_event);
    }
    if (event->type() == Event::Type::DisplayEvent) {
        DisplayEvent& own_event = *(DisplayEvent*)event.get();
        receive_display_event(own_event);
    }
    if (event->type() == Event::Type::LayoutEvent) {
        LayoutEvent& own_event = *(LayoutEvent*)event.get();
        receive_layout_event(own_event);
    }
}

} // namespace UI