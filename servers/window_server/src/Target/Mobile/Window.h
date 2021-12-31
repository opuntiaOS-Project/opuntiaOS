/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../Components/Base/BaseWindow.h"
#include "../../Components/MenuBar/MenuItem.h"
#include "../../IPC/Connection.h"
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>
#include <utility>

namespace WinServer::Mobile {

class Window : public BaseWindow {
public:
    Window(int connection_id, int id, CreateWindowMessage& msg);
    Window(Window&& win);

    inline void set_style(StatusBarStyle ts) { m_style = ts, on_style_change(); }
    inline StatusBarStyle style() { return m_style; }

private:
    void on_style_change();

    StatusBarStyle m_style { StatusBarStyle::StandardLight };
};

} // namespace WinServer