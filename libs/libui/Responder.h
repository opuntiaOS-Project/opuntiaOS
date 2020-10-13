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

class Responder : public LFoundation::EventReceiver {
public:
    bool send_invalidate_message(const LG::Rect& rect);

    void receive_event(UniquePtr<LFoundation::Event> event) override;
    virtual void receive_mouse_event(MouseEvent&) { }
    virtual void receive_display_event(DisplayEvent&) { }

protected:
    Responder() { }
};

}