/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libfoundation/Event.h>
#include <sys/types.h>

namespace UI {

class Event : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x2000,
        MouseEvent,
        Other,
    };

    Event(int type)
        : LFoundation::Event(type)
    {
    }

    ~Event() { }
};

class MouseEvent : public Event {
public:
    MouseEvent(uint32_t x, uint32_t y)
        : Event(Event::Type::MouseEvent)
        , m_x(x)
        , m_y(y)
    {
    }

    ~MouseEvent() { }

    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }

private:
    uint32_t m_x;
    uint32_t m_y;
};
}