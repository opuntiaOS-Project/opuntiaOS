/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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
        add_clip(view.frame_in_window());
    }
};
}