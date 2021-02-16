/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "View.h"
#include "Window.h"
#include <libg/Context.h>

namespace UI {
class Context : public LG::Context {
public:
    Context(LG::PixelBitmap& bitmap)
        : LG::Context(bitmap)
    {
    }

    Context(View& view)
        : Context(view.window()->bitmap())
    {
        auto frame = view.frame_in_window();
        add_clip(frame);
        set_draw_offset(frame.origin());
    }
};
}