/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Responder.h"
#include "App.h"
#include "Window.h"
#include <libfoundation/EventLoop.h>

namespace UI {

bool Responder::send_invalidate_message_to_server(const LG::Rect& rect) const
{
    auto& app = App::the();
    InvalidateMessage msg(Connection::the().key(), app.window().id(), rect);
    return app.connection().send_async_message(msg);
}

void Responder::send_display_message_to_self(Window& win, const LG::Rect& display_rect)
{
    if (!m_display_message_sent || m_prev_display_message != display_rect) {
        LFoundation::EventLoop::the().add(win, new DisplayEvent(display_rect));
        m_display_message_sent = true;
        m_prev_display_message = display_rect;
    }
}

void Responder::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::MouseEvent) {
        MouseEvent& own_event = *(MouseEvent*)event.get();
        receive_mouse_move_event(own_event);
    }
    if (event->type() == Event::Type::DisplayEvent) {
        DisplayEvent& own_event = *(DisplayEvent*)event.get();
        receive_display_event(own_event);
    }
}

}