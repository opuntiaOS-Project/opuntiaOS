/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include <libg/PixelBitmap.h>
#include <libg/Point.h>

class CursorManager {
public:
    static CursorManager& the();
    CursorManager();

    inline const LG::PixelBitmap& current_cursor() const { return std_cursor(); }
    inline const LG::PixelBitmap& std_cursor() const { return m_std_cursor; }
    inline LG::Point<int> draw_position(int mouse_x, int mouse_y) { return { mouse_x - 2, mouse_y - 2 }; }

private:
    LG::PixelBitmap m_std_cursor;
};