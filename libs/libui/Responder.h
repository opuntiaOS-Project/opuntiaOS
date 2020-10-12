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

namespace UI {

class Responder : public LFoundation::EventReceiver {
public:
    void receive_event(UniquePtr<LFoundation::Event> event) override
    {
        if (event->type() == Event::Type::MouseEvent) {
            receive_mouse_event(move(event));
        }
    }

    virtual void receive_mouse_event(UniquePtr<MouseEvent>) { }

protected:
    Responder() {}
};

}