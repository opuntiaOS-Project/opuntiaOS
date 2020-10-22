/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Responder.h"
#include "App.h"

namespace UI {

bool Responder::send_invalidate_message(const LG::Rect& rect)
{
    auto& app = App::the();
    InvalidateMessage msg(Connection::the().key(), app.window().id(), rect);
    return app.connection().send_async_message(msg);
}

void Responder::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::MouseEvent) {
        MouseEvent& own_event = *(MouseEvent*)event.get();
        receive_mouse_event(own_event);
    }
    if (event->type() == Event::Type::DisplayEvent) {
        DisplayEvent& own_event = *(DisplayEvent*)event.get();
        receive_display_event(own_event);
    }
}

}