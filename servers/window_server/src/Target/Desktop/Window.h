/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../Components/Base/BaseWindow.h"
#include "../../Components/MenuBar/MenuItem.h"
#include "../../IPC/Connection.h"
#include "WindowFrame.h"
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>
#include <utility>

namespace WinServer::Desktop {

class Window : public BaseWindow {
public:
    Window(int connection_id, int id, CreateWindowMessage& msg);
    Window(Window&& win);

    inline WindowFrame& frame() { return m_frame; }
    inline const WindowFrame& frame() const { return m_frame; }

    inline const LG::CornerMask& corner_mask() const { return m_corner_mask; }

    inline std::vector<MenuDir>& menubar_content() { return m_menubar_content; }
    inline const std::vector<MenuDir>& menubar_content() const { return m_menubar_content; }

    void on_menubar_change();

    virtual void did_app_title_change() override { m_frame.on_set_app_title(); }
    virtual void did_icon_path_change() override { m_frame.on_set_icon(); }
    virtual void did_size_change(const LG::Size& size) override;

    inline void set_style(StatusBarStyle style) { m_frame.set_style(style), on_style_change(); }

    void make_frame();
    void make_frameless();

private:
    void recalc_bounds(const LG::Size& size);
    void on_style_change();

    WindowFrame m_frame;
    LG::CornerMask m_corner_mask { LG::CornerMask::SystemRadius, LG::CornerMask::NonMasked, LG::CornerMask::Masked };
    std::vector<MenuDir> m_menubar_content;
};

} // namespace WinServer