/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Context.h"
#include <libcxx/syscalls.h>

namespace LG {

Context::Context(PixelBitmap& bitmap)
    : m_bitmap(bitmap)
    , m_origin_clip(0, 0, bitmap.width(), bitmap.height())
    , m_clip(0, 0, bitmap.width(), bitmap.height())
    , m_color()
{
}

void Context::add_clip(const Rect& rect)
{
    m_clip.intersect(rect);
}

void Context::reset_clip()
{
    m_clip = m_origin_clip;
}

void Context::draw(const Point<int>& start, const PixelBitmap& bitmap)
{
    Rect draw_bounds(start.x() + m_draw_offset.x(), start.y() + m_draw_offset.y(), bitmap.width(), bitmap.height());
    draw_bounds.intersect(m_clip);
    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int offset_x = -start.x();
    int offset_y = -start.y();
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int bitmap_x = x + offset_x;
            int bitmap_y = y + offset_y;
            m_bitmap[y][x] = bitmap[bitmap_y][bitmap_x];
        }
    }
}

void Context::draw(const Point<int>& start, const GlyphBitmap& bitmap)
{
    Rect draw_bounds(start.x() + m_draw_offset.x(), start.y() + m_draw_offset.y(), bitmap.width(), bitmap.height());
    draw_bounds.intersect(m_clip);
    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int offset_x = -start.x();
    int offset_y = -start.y();
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int bitmap_x = x + offset_x;
            int bitmap_y = y + offset_y;
            if (bitmap.bit_at(bitmap_x, bitmap_y)) {
                m_bitmap[y][x] = fill_color();
            }
        }
    }
}

void Context::fill(const Rect& rect)
{
    auto draw_bounds = rect;
    draw_bounds.offset_by(m_draw_offset);
    draw_bounds.intersect(m_clip);

    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            m_bitmap[y][x] = fill_color();
        }
    }
}

void Context::add_ellipse(const Rect& rect)
{
    int rx = rect.width() / 2;
    int ry = rect.height() / 2;
    int xc = rect.mid_x();
    int yc = rect.mid_y();

    double dx, dy, d1, d2, x, y;
    double tmp_d1, tmp_d2;
    x = 0;
    y = ry;

    d1 = (ry * ry) - (rx * rx * ry) + (0.25 * rx * rx);
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;

    while (dx < dy) {
        m_bitmap[y + yc][(int)x + xc] = fill_color();
        m_bitmap[y + yc][(int)-x + xc] = fill_color();
        m_bitmap[-y + yc][(int)x + xc] = fill_color();
        m_bitmap[-y + yc][(int)-x + xc] = fill_color();

        x++;
        dx += 2 * ry * ry;
        tmp_d1 = d1;
        d1 += ry * ry + dx;
        if (tmp_d1 >= 0) {
            y--;
            dy -= 2 * rx * rx;
            d1 -= dy;
        }
    }

    d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) + ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

    while (y >= 0) {
        m_bitmap[y + yc][(int)x + xc] = fill_color();
        m_bitmap[y + yc][(int)-x + xc] = fill_color();
        m_bitmap[-y + yc][(int)x + xc] = fill_color();
        m_bitmap[-y + yc][(int)-x + xc] = fill_color();

        y--;
        dy -= 2 * rx * rx;
        tmp_d2 = d2;
        d2 += rx * rx - dy;
        if (tmp_d2 <= 0) {
            x++;
            dx += 2 * ry * ry;
            d2 += dx;
        }
    }
}

}