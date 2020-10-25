/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Event.h"
#include <libfoundation/EventReceiver.h>
#include <libg/Rect.h>

namespace UI {

class Window;

class Responder : public LFoundation::EventReceiver {
public:
    bool send_invalidate_message_to_server(const LG::Rect& rect) const;
    void send_display_message_to_self(Window& win, const LG::Rect& display_rect);

    void receive_event(UniquePtr<LFoundation::Event> event) override;
    virtual void receive_mouse_move_event(MouseEvent&) { }
    virtual void receive_mouse_action_event(MouseActionEvent&) { }
    virtual void receive_mouse_leave_event(MouseLeaveEvent&) { }
    virtual void receive_display_event(DisplayEvent&) { m_display_message_sent = false; }

protected:
    bool m_display_message_sent { false };
    LG::Rect m_prev_display_message {};
    Responder() { }
};

}