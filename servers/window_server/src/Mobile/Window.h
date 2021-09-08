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
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>
#include <utility>

namespace WinServer::Mobile {

class Window : public BaseWindow {
public:
    Window(int connection_id, int id, const CreateWindowMessage& msg);
    Window(Window&& win);

    inline void set_style(const LG::Color& clr, TextStyle ts) { m_color = clr, m_text_style = ts, on_style_change(); }
    inline LG::Color& color() { return m_color; }
    inline const LG::Color& color() const { return m_color; }

    inline TextStyle text_style() { return m_text_style; }

private:
    void on_style_change();

    LG::Color m_color { LG::Color::LightSystemBackground };
    TextStyle m_text_style { TextStyle::Dark };
};

} // namespace WinServer