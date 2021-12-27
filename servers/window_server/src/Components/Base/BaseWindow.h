/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../shared/MessageContent/MenuBar.h"
#include "../../../shared/MessageContent/Window.h"
#include "../../IPC/Connection.h"
#include "../MenuBar/MenuItem.h"
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <libg/Size.h>
#include <sys/types.h>
#include <utility>

namespace WinServer {

typedef uint32_t WindowEventMask;
enum WindowEvent {
    WindowStatus = (1 << 0),
    WindowCreation = (1 << 1),
    IconChange = (1 << 2),
    TitleChange = (1 << 3),
};

class BaseWindow {
public:
    BaseWindow(int connection_id, int id, const CreateWindowMessage& msg);
    BaseWindow(BaseWindow&& win);
    ~BaseWindow() = default;

    void set_buffer(int buffer_id, LG::Size sz, LG::PixelBitmapFormat fmt);

    inline int id() const { return m_id; }
    inline int connection_id() const { return m_connection_id; }
    inline WindowType type() const { return m_type; }
    inline WindowEventMask event_mask() const { return m_event_mask; }
    inline void set_event_mask(WindowEventMask mask) { m_event_mask = mask; }

    inline LFoundation::SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    inline LG::PixelBitmap& content_bitmap() { return m_content_bitmap; }
    inline const LG::PixelBitmap& content_bitmap() const { return m_content_bitmap; }

    inline LG::Rect& content_bounds() { return m_content_bounds; }
    inline const LG::Rect& content_bounds() const { return m_content_bounds; }

    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

    inline bool visible() const { return m_visible; }
    inline void set_visible(bool vis) { m_visible = vis; }

    inline void set_needs_display(const LG::Rect& rect) const
    {
        DisplayMessage msg(connection_id(), rect);
        Connection::the().send_async_message(msg);
    }

    inline void offset_by(int x_offset, int y_offset)
    {
        bounds().offset_by(x_offset, y_offset);
        content_bounds().offset_by(x_offset, y_offset);
    }

    virtual void did_size_change(const LG::Size& size) { }

protected:
    int m_id { -1 };
    int m_connection_id { -1 };
    bool m_visible { true };
    WindowType m_type { WindowType::Standard };
    WindowEventMask m_event_mask { 0 };
    LG::Rect m_bounds;
    LG::Rect m_content_bounds;
    LG::PixelBitmap m_content_bitmap;
    LFoundation::SharedBuffer<LG::Color> m_buffer;
};

} // namespace WinServer