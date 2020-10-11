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

static void _log_hex(uint32_t hex)
{
    uint32_t pk = (uint32_t)0x10000000;
    bool was_not_zero = 0;

    if (hex == 0) {
        write(1, "0", 1);
    } else {
        write(1, "0x", 2);
    }

    while (pk > 0) {
        uint32_t pp = hex / pk;
        if (was_not_zero || pp > 0) {
            char oo = pp - 10 + 'A';
            if (pp >= 10) {
            } else {
                oo = pp + '0';
            }
            write(1, &oo, 1);
            was_not_zero = 1;
        }
        hex -= pp * pk;
        pk /= 16;
    }
}

void Context::draw(const Point<int>& start, const PixelBitmap& bitmap)
{
    Rect draw_bounds(start.x(), start.y(), bitmap.width(), bitmap.height());
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
            int bitmap_x = min_x + offset_x;
            int bitmap_y = min_y + offset_y;
            m_bitmap[y][x] = bitmap[bitmap_y][bitmap_x];
        }
    }
}

void Context::fill(const Rect& rect)
{
    auto draw_bounds = rect;
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
            m_bitmap[y][x] = 0x0;
        }
    }
}

}