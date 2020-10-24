/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libfoundation/Event.h>
#include <libg/Rect.h>
#include <sys/types.h>

namespace UI {

class Event : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x2000,
        MouseEvent,
        MouseActionEvent,
        MouseLeaveEvent,
        DisplayEvent,
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

enum MouseActionType {
    ClickBegan,
    ClickEnded,
};

class MouseActionEvent : public Event {
public:
    MouseActionEvent(MouseActionType type, uint32_t x, uint32_t y)
        : Event(Event::Type::MouseActionEvent)
        , m_type(type)
        , m_x(x)
        , m_y(y)
    {
    }

    ~MouseActionEvent() { }

    MouseActionType type() const { return m_type; }
    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }

private:
    MouseActionType m_type;
    uint32_t m_x;
    uint32_t m_y;
};

class MouseLeaveEvent : public Event {
public:
    MouseLeaveEvent(uint32_t x, uint32_t y)
        : Event(Event::Type::MouseLeaveEvent)
        , m_x(x)
        , m_y(y)
    {
    }

    ~MouseLeaveEvent() { }

    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }

private:
    uint32_t m_x;
    uint32_t m_y;
};

class DisplayEvent : public Event {
public:
    DisplayEvent(const LG::Rect& rect)
        : Event(Event::Type::DisplayEvent)
        , m_display_bounds(rect)
    {
    }

    ~DisplayEvent() { }

    LG::Rect& bounds() { return m_display_bounds; }
    const LG::Rect& bounds() const { return m_display_bounds; }

private:
    LG::Rect m_display_bounds;
};

}