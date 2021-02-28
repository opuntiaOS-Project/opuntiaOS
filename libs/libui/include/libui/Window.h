/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../servers/window_server/WSConnection.h"
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/String.h>
#include <libui/View.h>
#include <sys/SharedBuffer.h>
#include <sys/types.h>

namespace UI {

enum WindowType {
    Standard = 0,
    Dock = 1,
};

class Window : public LFoundation::EventReceiver {
public:
    Window();
    Window(uint32_t width, uint32_t height, WindowType type = WindowType::Standard);
    Window(uint32_t width, uint32_t height, const LG::String& path);

    int id() const { return m_id; }
    inline WindowType type() const { return m_type; }

    SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

    LG::PixelBitmap& bitmap() { return m_bitmap; }
    const LG::PixelBitmap& bitmap() const { return m_bitmap; }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

    template <class T, class... Args>
    inline T& create_superview(Args&&... args)
    {
        T* new_view = new T(bounds(), args...);
        new_view->set_window(this);
        m_superview = new_view;
        m_superview->set_needs_display();
        return *new_view;
    }

    inline void set_bitmap_format(LG::PixelBitmapFormat format) { m_bitmap.set_format(format), did_format_change(); }

    inline View* superview() const { return m_superview; }

    inline void set_focused_view(View& view) { m_focused_view = &view; }
    inline View* focused_view() { return m_focused_view; }

    inline const LG::Rect& bounds() const { return m_bounds; }

    bool set_title(const LG::String& title);
    bool set_frame_style(const LG::Color& color);
    bool did_format_change();

    inline const LG::String& icon_path() const { return m_icon_path; }

private:
    uint32_t m_id;
    WindowType m_type { WindowType::Standard };
    LG::Rect m_bounds;
    LG::PixelBitmap m_bitmap;
    SharedBuffer<LG::Color> m_buffer;
    View* m_superview { nullptr };
    View* m_focused_view { nullptr };
    LG::String m_icon_path { "/res/icons/apps/missing.icon" };
};

}