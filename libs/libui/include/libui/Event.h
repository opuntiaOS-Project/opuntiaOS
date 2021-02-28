/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/Event.h>
#include <libg/Rect.h>
#include <libg/String.h>
#include <sys/types.h>

namespace UI {

class Event : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x2000,
        MouseEvent,
        MouseActionEvent,
        MouseLeaveEvent,
        KeyUpEvent,
        KeyDownEvent,
        DisplayEvent,
        WindowCloseRequestEvent,

        NotifyWindowStatusChangedEvent,
        NotifyWindowIconChangedEvent,
        Other,
    };

    explicit Event(int type)
        : LFoundation::Event(type)
    {
    }

    ~Event() = default;
};

class MouseEvent : public Event {
public:
    MouseEvent(uint32_t x, uint32_t y)
        : Event(Event::Type::MouseEvent)
        , m_x(x)
        , m_y(y)
    {
    }

    ~MouseEvent() = default;

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

    ~MouseActionEvent() = default;

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

    ~MouseLeaveEvent() = default;

    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }

private:
    uint32_t m_x;
    uint32_t m_y;
};

typedef uint32_t key_t;
class KeyUpEvent : public Event {
public:
    KeyUpEvent(key_t key)
        : Event(Event::Type::KeyUpEvent)
        , m_key(key)
    {
    }

    ~KeyUpEvent() = default;

    key_t key() const { return m_key; }

private:
    key_t m_key;
};

class KeyDownEvent : public Event {
public:
    KeyDownEvent(key_t key)
        : Event(Event::Type::KeyDownEvent)
        , m_key(key)
    {
    }

    ~KeyDownEvent() = default;

    key_t key() const { return m_key; }

private:
    key_t m_key;
};

class DisplayEvent : public Event {
public:
    DisplayEvent(const LG::Rect& rect)
        : Event(Event::Type::DisplayEvent)
        , m_display_bounds(rect)
    {
    }

    ~DisplayEvent() = default;

    LG::Rect& bounds() { return m_display_bounds; }
    const LG::Rect& bounds() const { return m_display_bounds; }

private:
    LG::Rect m_display_bounds;
};

class WindowCloseRequestEvent : public Event {
public:
    WindowCloseRequestEvent(uint32_t window_id)
        : Event(Event::Type::WindowCloseRequestEvent)
        , m_window_id(window_id)
    {
    }

    ~WindowCloseRequestEvent() = default;
    uint32_t window_id() const { return m_window_id; }

private:
    uint32_t m_window_id;
};

// Notifiers
class NotifyWindowStatusChangedEvent : public Event {
public:
    NotifyWindowStatusChangedEvent(uint32_t changed_window_id, int type)
        : Event(Event::Type::NotifyWindowStatusChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_type(type)
    {
    }

    ~NotifyWindowStatusChangedEvent() = default;
    uint32_t changed_window_id() const { return m_changed_window_id; }
    int type() const { return m_type; }

private:
    uint32_t m_changed_window_id;
    int m_type;
};

class NotifyWindowIconChangedEvent : public Event {
public:
    NotifyWindowIconChangedEvent(uint32_t changed_window_id, const LG::String& path)
        : Event(Event::Type::NotifyWindowIconChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_icon_path(path)
    {
    }

    NotifyWindowIconChangedEvent(uint32_t changed_window_id, LG::String&& path)
        : Event(Event::Type::NotifyWindowIconChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_icon_path(std::move(path))
    {
    }

    ~NotifyWindowIconChangedEvent() = default;
    uint32_t changed_window_id() const { return m_changed_window_id; }
    const LG::String& icon_path() const { return m_icon_path; }

private:
    uint32_t m_changed_window_id;
    LG::String m_icon_path;
};

}