/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../Components/Base/BaseWindow.h"
#include "../Components/MenuBar/MenuItem.h"
#include "../Connection.h"
#include "WindowFrame.h"
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>
#include <utility>

namespace WinServer::Desktop {

class Window : public BaseWindow {
public:
    Window(int connection_id, int id, const CreateWindowMessage& msg);
    Window(Window&& win);

    inline WindowFrame& frame() { return m_frame; }
    inline const WindowFrame& frame() const { return m_frame; }

    inline void set_name(const std::string& name)
    {
        frame().set_app_name(name);
        menubar_content()[0].set_title(name);
    }

    inline void offset_by(int x_offset, int y_offset)
    {
        bounds().offset_by(x_offset, y_offset);
        content_bounds().offset_by(x_offset, y_offset);
    }

    void make_frame();
    void make_frameless();

    inline void set_icon(LG::string&& name)
    {
        m_icon_path = std::move(name);
        m_frame.reload_icon();
    }

    inline void set_icon(const LG::string& name)
    {
        m_icon_path = name;
        m_frame.reload_icon();
    }

    inline const LG::CornerMask& corner_mask() const { return m_corner_mask; }

    inline const LG::string& icon_path() const { return m_icon_path; }

    inline std::vector<MenuDir>& menubar_content() { return m_menubar_content; }
    inline const std::vector<MenuDir>& menubar_content() const { return m_menubar_content; }

    virtual void did_size_change(const LG::Size& size) override;

private:
    void recalc_bounds(const LG::Size& size);

    WindowFrame m_frame;
    LG::CornerMask m_corner_mask { LG::CornerMask::SystemRadius, LG::CornerMask::NonMasked, LG::CornerMask::Masked };
    LG::string m_icon_path {};
    std::vector<MenuDir> m_menubar_content;
};

} // namespace WinServer