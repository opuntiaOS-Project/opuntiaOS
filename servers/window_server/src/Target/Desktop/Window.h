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

    inline const std::string& app_name() const { return m_app_name; }
    inline const std::string& bundle_id() const { return m_bundle_id; }

    inline const std::string& app_title() const { return m_app_title; }
    inline void set_app_title(const std::string& title) { m_app_title = title, m_frame.on_set_app_title(); }

    inline const std::string& icon_path() const { return m_icon_path; }
    inline void set_icon_path(std::string&& name) { m_icon_path = std::move(name), m_frame.on_set_icon(); }
    inline void set_icon_path(const std::string& name) { m_icon_path = name, m_frame.on_set_icon(); }

    inline const LG::CornerMask& corner_mask() const { return m_corner_mask; }

    inline std::vector<MenuDir>& menubar_content() { return m_menubar_content; }
    inline const std::vector<MenuDir>& menubar_content() const { return m_menubar_content; }

    void on_menubar_change();

    virtual void did_size_change(const LG::Size& size) override;

    inline void set_style(StatusBarStyle style) { m_frame.set_style(style), on_style_change(); }

    void make_frame();
    void make_frameless();

private:
    void recalc_bounds(const LG::Size& size);
    void on_style_change();

    WindowFrame m_frame;
    LG::CornerMask m_corner_mask { LG::CornerMask::SystemRadius, LG::CornerMask::NonMasked, LG::CornerMask::Masked };
    std::string m_app_name {};
    std::string m_app_title {};
    std::string m_icon_path {};
    std::string m_bundle_id {};
    std::vector<MenuDir> m_menubar_content;
};

} // namespace WinServer