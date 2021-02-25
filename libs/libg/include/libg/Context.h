/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libg/Color.h>
#include <libg/Font.h>
#include <libg/PixelBitmap.h>
#include <libg/Point.h>
#include <libg/Rect.h>
#include <libg/Shading.h>
#include <sys/types.h>

namespace LG {

class Context {
public:
    explicit Context(PixelBitmap&);
    ~Context() = default;

    void add_clip(const Rect& rect);
    void reset_clip();

    void set(const Point<int>& start, const PixelBitmap& bitmap);
    void draw(const Point<int>& start, const PixelBitmap& bitmap);
    void draw(const Point<int>& start, const GlyphBitmap& bitmap);
    void draw_shading(const Rect& rect, const Shading& shading);
    void fill(const Rect& rect);
    void mix(const Rect& rect);
    void add_ellipse(const Rect& rect);

    void set_draw_offset(const Point<int>& offset) { m_draw_offset = offset; }
    void set_fill_color(const Color& clr) { m_color = clr; }

    inline const Color& fill_color() const { return m_color; }

private:
    PixelBitmap& m_bitmap;
    Rect m_clip;
    const Rect m_origin_clip;
    Point<int> m_draw_offset { 0, 0 };
    Color m_color {};
};

}