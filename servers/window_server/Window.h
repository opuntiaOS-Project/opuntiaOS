/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

enum WindowType : int {
    Standard = 0,
    Dock = 1,
};

enum WindowStatusUpdateType : int {
    Created,
    Changed,
    Removed,
};

typedef uint32_t WindowEventMask;
enum WindowEvent {
    WindowStatus = (1 << 0),
    IconChange = (1 << 1),
};

class Window : public Weakable<Window>, public LinkedListNode<Window> {
public:
    Window(int connection_id, int id, const CreateWindowMessage& msg);
    Window(Window&& win);

    void set_buffer(int buffer_id);

    inline int id() const { return m_id; }
    inline int connection_id() const { return m_connection_id; }
    inline WindowType type() const { return m_type; }
    inline WindowEventMask event_mask() const { return m_event_mask; }
    inline void set_event_mask(WindowEventMask mask) { m_event_mask = mask; }

    inline SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    inline LG::PixelBitmap& content_bitmap() { return m_content_bitmap; }
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

    void make_frame();
    void make_frameless();

    inline void set_icon(LG::String&& name)
    {
        m_icon_path = move(name);
        m_frame.reload_icon();
    }

    inline void set_icon(const LG::String& name)
    {
        m_icon_path = name;
        m_frame.reload_icon();
    }

    inline const LG::String& icon_path() const { return m_icon_path; }

private:
    int m_id { -1 };
    int m_connection_id { -1 };
    WindowType m_type { WindowType::Standard };
    WindowEventMask m_event_mask { 0 };
    LG::Rect m_bounds;
    LG::Rect m_content_bounds;
    LG::PixelBitmap m_content_bitmap;
    SharedBuffer<LG::Color> m_buffer;
    WindowFrame m_frame;
    LG::String m_icon_path {};
};
