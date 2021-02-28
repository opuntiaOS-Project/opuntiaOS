/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/EventReceiver.h>
#include <libg/Rect.h>
#include <libui/Event.h>

namespace UI {

class Window;

class Responder : public LFoundation::EventReceiver {
public:
    bool send_invalidate_message_to_server(const LG::Rect& rect) const;
    void send_display_message_to_self(Window& win, const LG::Rect& display_rect);

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;
    virtual void receive_mouse_move_event(MouseEvent&) { }
    virtual void receive_mouse_action_event(MouseActionEvent&) { }
    virtual void receive_mouse_leave_event(MouseLeaveEvent&) { }
    virtual void receive_keyup_event(KeyUpEvent&) { }
    virtual void receive_keydown_event(KeyDownEvent&) { }
    virtual void receive_display_event(DisplayEvent&) { m_display_message_sent = false; }

protected:
    bool m_display_message_sent { false };
    LG::Rect m_prev_display_message {};
    Responder() = default;
};

}