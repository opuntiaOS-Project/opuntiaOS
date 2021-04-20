/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Context.h>

namespace WinServer {

enum class WidgetAnswer {
    Empty,
    InvalidateMe,
    Bad,
};

class BaseWidget {
public:
    BaseWidget() = default;
    virtual ~BaseWidget() = default;

    size_t height() { return 20; }
    virtual size_t width() { return 0; }
    virtual void draw(LG::Context& ctx) { }
    virtual WidgetAnswer click_began(int x, int y) { return WidgetAnswer::Empty; }
    virtual WidgetAnswer click_ended() { return WidgetAnswer::Empty; }
};

} // namespace WinServer