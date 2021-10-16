/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../servers/window_server/shared/Connections/WSConnection.h"
#include "../../../servers/window_server/shared/MessageContent/Window.h"
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libfoundation/SharedBuffer.h>
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/Size.h>
#include <string>
#include <libui/MenuBar.h>
#include <libui/PopupMenu.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <sys/types.h>

namespace UI {

using ::WindowStatusUpdateType;
using ::WindowType;

class Connection;

class Window : public LFoundation::EventReceiver {
    UI_OBJECT();
    friend Connection;

public:
    Window(const std::string& title, const LG::Size& size, WindowType type = WindowType::Standard);
    Window(const std::string& title, const LG::Size& size, const std::string& path);
    Window(const std::string& title, const LG::Size& size, const std::string& path, const StatusBarStyle& style);

    int id() const { return m_id; }
    inline WindowType type() const { return m_type; }

    inline const LG::Rect& bounds() const { return m_bounds; }
    LFoundation::SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const LFoundation::SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

    LG::PixelBitmap& bitmap() { return m_bitmap; }
    const LG::PixelBitmap& bitmap() const { return m_bitmap; }
    inline void set_bitmap_format(LG::PixelBitmapFormat format) { m_bitmap.set_format(format), did_format_change(); }

    template <class ViewT, class ViewControllerT, class... Args>
    inline ViewT& create_superview(Args&&... args)
    {
        ViewT* new_view = new ViewT(nullptr, this, bounds(), args...);
        m_superview = new_view;
        m_root_view_controller = new ViewControllerT(*new_view);

        setup_superview();
        m_superview->set_needs_display();
        LFoundation::EventLoop::the().add(*m_root_view_controller, new ViewDidLoadEvent());
        return *new_view;
    }
    inline View* superview() const { return m_superview; }

    inline void set_focused_view(View& view) { m_focused_view = &view; }
    inline View* focused_view() { return m_focused_view; }

    MenuBar& menubar() { return m_menubar; }
    PopupMenu& popup_manager() { return m_popup; }

    bool set_title(const std::string& title);
    bool set_status_bar_style(StatusBarStyle style);
    bool did_format_change();
    bool did_buffer_change();

    inline const std::string& title() const { return m_title; }
    inline const std::string& icon_path() const { return m_icon_path; }
    inline const StatusBarStyle& status_bar_style() const { return m_status_bar_style; }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

private:
    void resize(ResizeEvent&);
    void setup_superview();
    void fill_with_opaque(const LG::Rect&);

    uint32_t m_id;
    BaseViewController* m_root_view_controller { nullptr };
    View* m_superview { nullptr };
    View* m_focused_view { nullptr };

    LG::Rect m_bounds;
    LG::PixelBitmap m_bitmap;
    LFoundation::SharedBuffer<LG::Color> m_buffer;
    std::string m_title { "" };
    std::string m_icon_path { "/res/icons/apps/missing.icon" };
    LG::Color m_color;
    StatusBarStyle m_status_bar_style;
    WindowType m_type { WindowType::Standard };

    MenuBar m_menubar;
    PopupMenu m_popup;
};

} // namespace UI