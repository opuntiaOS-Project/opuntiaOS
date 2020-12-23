/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Connection.h"
#include "WindowFrame.h"
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <std/LinkedList.h>
#include <std/Utility.h>
#include <std/WeakPtr.h>
#include <sys/SharedBuffer.h>
#include <sys/types.h>

class Window : public Weakable<Window>, public LinkedListNode<Window> {
public:
    Window(int connection_id, int id, const CreateWindowMessage& msg);
    Window(Window&& win);

    void set_buffer(int buffer_id);

    inline int id() const { return m_id; }

    inline SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    inline const LG::PixelBitmap& content_bitmap() const { return m_content_bitmap; }

    inline LG::Rect& content_bounds() { return m_content_bounds; }
    inline const LG::Rect& content_bounds() const { return m_content_bounds; }

    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

    inline WindowFrame& frame() { return m_frame; }
    inline const WindowFrame& frame() const { return m_frame; }

    inline void set_needs_display(const LG::Rect& rect) const
    {
        DisplayMessage msg(connection_id(), rect);
        Connection::the().send_async_message(msg);
    }

    inline int connection_id() const { return m_connection_id; }

    void will_be_closed();
    void will_be_minimized();
    void was_minimized();
    void will_be_maximized();
    void was_maximized();

private:
    int m_id { -1 };
    int m_connection_id { -1 };
    LG::Rect m_bounds;
    LG::Rect m_content_bounds;
    LG::PixelBitmap m_content_bitmap;
    SharedBuffer<LG::Color> m_buffer;
    WindowFrame m_frame;
};
