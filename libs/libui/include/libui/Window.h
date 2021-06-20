/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../servers/window_server/shared/Connections/WSConnection.h"
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libfoundation/SharedBuffer.h>
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/Size.h>
#include <libg/string.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <sys/types.h>

namespace UI {

enum WindowType {
    Standard = 0,
    Homescreen = 1,
};

class Connection;

class Window : public LFoundation::EventReceiver {
    UI_OBJECT();
    friend Connection;

public:
    Window(const LG::Size& size, WindowType type = WindowType::Standard);
    Window(const LG::Size& size, const LG::string& path);

    int id() const { return m_id; }
    inline WindowType type() const { return m_type; }

    LFoundation::SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const LFoundation::SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

    LG::PixelBitmap& bitmap() { return m_bitmap; }
    const LG::PixelBitmap& bitmap() const { return m_bitmap; }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

    template <class ViewT, class ViewControllerT, class... Args>
    inline ViewT& create_superview(Args&&... args)
    {
        ViewT* new_view = new ViewT(nullptr, this, bounds(), args...);
        m_superview = new_view;
        m_root_view_controller = new ViewControllerT(*new_view);

        m_superview->set_needs_display();
        LFoundation::EventLoop::the().add(*m_root_view_controller, new ViewDidLoadEvent());
        return *new_view;
    }

    inline void set_bitmap_format(LG::PixelBitmapFormat format) { m_bitmap.set_format(format), did_format_change(); }

    inline View* superview() const { return m_superview; }

    inline void set_focused_view(View& view) { m_focused_view = &view; }
    inline View* focused_view() { return m_focused_view; }

    inline const LG::Rect& bounds() const { return m_bounds; }

    bool set_title(const LG::string& title);
    bool set_frame_style(const LG::Color& color);
    bool did_format_change();

    inline const LG::string& icon_path() const { return m_icon_path; }

private:
    uint32_t m_id;
    BaseViewController* m_root_view_controller { nullptr };
    WindowType m_type { WindowType::Standard };
    LG::Rect m_bounds;
    LG::PixelBitmap m_bitmap;
    LFoundation::SharedBuffer<LG::Color> m_buffer;
    View* m_superview { nullptr };
    View* m_focused_view { nullptr };
    LG::string m_icon_path { "/res/icons/apps/missing.icon" };
};

} // namespace UI