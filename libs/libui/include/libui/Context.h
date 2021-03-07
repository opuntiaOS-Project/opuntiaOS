/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Context.h>
#include <libui/View.h>
#include <libui/Window.h>

namespace UI {
class Context : public LG::Context {
public:
    explicit Context(LG::PixelBitmap& bitmap)
        : LG::Context(bitmap)
    {
    }

    explicit Context(View& view)
        : Context(view.window()->bitmap())
    {
        auto frame = view.frame_in_window();
        add_clip(frame);
        set_draw_offset(frame.origin());
    }
};
} // namespace UI