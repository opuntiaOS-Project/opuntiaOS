/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../Helpers/TextDrawer.h"
#include "../../MenuBar.h"
#include "../BaseWidget.h"
#include <ctime>
#include <libfoundation/Logger.h>
#include <libg/Font.h>
#include <libg/ImageLoaders/PNGLoader.h>

#define DATA_BUF 32

namespace WinServer {

class ControlPanelToggle : public BaseWidget {
public:
    ControlPanelToggle()
    {
        LG::PNG::PNGLoader loader;
        m_icon = loader.load_from_file("/res/system/contol_center.png");
    }

    ~ControlPanelToggle() = default;

    constexpr size_t width() override { return 20; }
    constexpr size_t height() { return 12; }
    void draw(LG::Context& ctx) override
    {
        ctx.draw({ 4, 4 }, m_icon);
    }

    MenuItemAnswer mouse_down(int x, int y) override
    {
        m_clicked = true;
        return MenuItemAnswer::InvalidateMe;
    }

    MenuItemAnswer mouse_up() override
    {
        m_clicked = false;
        return MenuItemAnswer::InvalidateMe;
    }

    void popup_rect(LG::Rect& r) override { }

private:
    LG::PixelBitmap m_icon;
    bool m_clicked { false };
};

} // namespace WinServer