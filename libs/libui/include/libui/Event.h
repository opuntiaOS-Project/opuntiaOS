/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../../../../servers/window_server/shared/MessageContent/MouseAction.h"
#include <libfoundation/Event.h>
#include <libg/Rect.h>
#include <string>
#include <sys/types.h>

namespace UI {

class Event : public LFoundation::Event {
public:
    // Some of events can be sent and are used in processing of data,
    // while some part of events are used only for describing actions.
    enum Type {
        Invalid = 0x2000,
        MouseEvent,
        MouseUpEvent,
        MouseDownEvent,
        MouseActionEvent,
        MouseEnterEvent,
        MouseLeaveEvent,
        MouseWheelEvent,
        KeyUpEvent,
        KeyDownEvent,
        DisplayEvent,
        LayoutEvent,
        WindowCloseRequestEvent,
        ResizeEvent,
        MenuBarActionEvent,
        PopupActionEvent,

        UIHandlerInvoke,

        NotifyWindowCreateEvent,
        NotifyWindowStatusChangedEvent,
        NotifyWindowIconChangedEvent,
        NotifyWindowTitleChangedEvent,

        ViewDidLoad,

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

class MouseWheelEvent : public Event {
public:
    MouseWheelEvent(uint32_t x, uint32_t y, int data)
        : Event(Event::Type::MouseWheelEvent)
        , m_x(x)
        , m_y(y)
        , m_wheel_data(data)
    {
    }

    ~MouseWheelEvent() = default;

    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }
    int wheel_data() const { return m_wheel_data; }

private:
    uint32_t m_x;
    uint32_t m_y;
    int m_wheel_data;
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

class View;
class LayoutEvent : public Event {
public:
    LayoutEvent(View* rect)
        : Event(Event::Type::LayoutEvent)
        , m_target(rect)
    {
    }

    ~LayoutEvent() = default;

    View* target() const { return m_target; }

private:
    View* m_target;
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

class ResizeEvent : public Event {
public:
    ResizeEvent(uint32_t window_id, const LG::Rect& bounds)
        : Event(Event::Type::ResizeEvent)
        , m_window_id(window_id)
        , m_bounds(bounds)
    {
    }

    ~ResizeEvent() = default;
    uint32_t window_id() const { return m_window_id; }
    const LG::Rect& bounds() const { return m_bounds; }

private:
    uint32_t m_window_id;
    LG::Rect m_bounds;
};

class MenuBarActionEvent : public Event {
public:
    MenuBarActionEvent(uint32_t window_id, int menu_id, int item_id)
        : Event(Event::Type::MenuBarActionEvent)
        , m_window_id(window_id)
        , m_menu_id(menu_id)
        , m_item_id(item_id)
    {
    }

    ~MenuBarActionEvent() = default;
    uint32_t window_id() const { return m_window_id; }
    int menu_id() const { return m_menu_id; }
    int item_id() const { return m_item_id; }

private:
    uint32_t m_window_id;
    int m_menu_id;
    int m_item_id;
};

class PopupActionEvent : public Event {
public:
    PopupActionEvent(uint32_t window_id, int menu_id, int item_id)
        : Event(Event::Type::PopupActionEvent)
        , m_window_id(window_id)
        , m_menu_id(menu_id)
        , m_item_id(item_id)
    {
    }

    ~PopupActionEvent() = default;
    uint32_t window_id() const { return m_window_id; }
    int menu_id() const { return m_menu_id; }
    int item_id() const { return m_item_id; }

private:
    uint32_t m_window_id;
    int m_menu_id;
    int m_item_id;
};

// Notifiers
class NotifyWindowCreateEvent : public Event {
public:
    NotifyWindowCreateEvent(std::string&& bundle_id, std::string&& icon_path, uint32_t changed_window_id)
        : Event(Event::Type::NotifyWindowCreateEvent)
        , m_window_id(changed_window_id)
        , m_icon_path(icon_path)
        , m_bundle_id(bundle_id)
    {
    }

    ~NotifyWindowCreateEvent() = default;
    uint32_t window_id() const { return m_window_id; }
    const std::string& icon_path() const { return m_icon_path; }
    const std::string& bundle_id() const { return m_bundle_id; }

private:
    int m_window_id;
    std::string m_bundle_id;
    std::string m_icon_path;
};

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
    NotifyWindowIconChangedEvent(uint32_t changed_window_id, const std::string& path)
        : Event(Event::Type::NotifyWindowIconChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_icon_path(path)
    {
    }

    NotifyWindowIconChangedEvent(uint32_t changed_window_id, std::string&& path)
        : Event(Event::Type::NotifyWindowIconChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_icon_path(std::move(path))
    {
    }

    ~NotifyWindowIconChangedEvent() = default;
    uint32_t changed_window_id() const { return m_changed_window_id; }
    const std::string& icon_path() const { return m_icon_path; }

private:
    uint32_t m_changed_window_id;
    std::string m_icon_path;
};

class NotifyWindowTitleChangedEvent : public Event {
public:
    NotifyWindowTitleChangedEvent(uint32_t changed_window_id, const std::string& title)
        : Event(Event::Type::NotifyWindowTitleChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_title(title)
    {
    }

    NotifyWindowTitleChangedEvent(uint32_t changed_window_id, std::string&& title)
        : Event(Event::Type::NotifyWindowTitleChangedEvent)
        , m_changed_window_id(changed_window_id)
        , m_title(std::move(title))
    {
    }

    ~NotifyWindowTitleChangedEvent() = default;
    uint32_t changed_window_id() const { return m_changed_window_id; }
    const std::string& title() const { return m_title; }

private:
    uint32_t m_changed_window_id;
    std::string m_title;
};

// View Life Cycle Events
class ViewDidLoadEvent : public Event {
public:
    ViewDidLoadEvent()
        : Event(Event::Type::ViewDidLoad)
    {
    }

    ~ViewDidLoadEvent() = default;

private:
};

} // namespace UI