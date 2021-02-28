/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <libg/Context.h>
#include <std/Memory.h>

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

void Context::set(const Point<int>& start, const PixelBitmap& bitmap)
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
    int offset_x = -start.x() - m_draw_offset.x();
    int offset_y = -start.y() - m_draw_offset.y();
    int bitmap_x = min_x + offset_x;
    int bitmap_y = min_y + offset_y;
    int len_x = max_x - min_x + 1;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        fast_copy((uint32_t*)&m_bitmap[y][min_x], (uint32_t*)&bitmap[bitmap_y][bitmap_x], len_x);
    }
}

void Context::draw(const Point<int>& start, const PixelBitmap& bitmap)
{
    if (!bitmap.has_alpha_channel()) {
        set(start, bitmap);
        return;
    }

    Rect draw_bounds(start.x() + m_draw_offset.x(), start.y() + m_draw_offset.y(), bitmap.width(), bitmap.height());
    draw_bounds.intersect(m_clip);
    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int offset_x = -start.x() - m_draw_offset.x();
    int offset_y = -start.y() - m_draw_offset.y();
    int bitmap_y = min_y + offset_y;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        int bitmap_x = min_x + offset_x;
        for (int x = min_x; x <= max_x; x++, bitmap_x++) {
            m_bitmap[y][x].mix_with(bitmap[bitmap_y][bitmap_x]);
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

    auto& color = fill_color();
    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int offset_x = -start.x() - m_draw_offset.x();
    int offset_y = -start.y() - m_draw_offset.y();
    int bitmap_y = min_y + offset_y;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        int bitmap_x = min_x + offset_x;
        for (int x = min_x; x <= max_x; x++, bitmap_x++) {
            if (bitmap.bit_at(bitmap_x, bitmap_y)) {
                m_bitmap[y][x] = color;
            }
        }
    }
}

void Context::mix(const Rect& rect)
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
    const auto& color = fill_color();
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            m_bitmap[y][x].mix_with(color);
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

    auto color = fill_color().u32();
    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int len_x = max_x - min_x + 1;
    for (int y = min_y; y <= max_y; y++) {
        fast_set((uint32_t*)&m_bitmap[y][min_x], color, len_x);
    }
}

void Context::draw_shading(const Rect& rect, const Shading& shading)
{
    auto draw_bounds = rect;
    draw_bounds.offset_by(m_draw_offset);
    auto orig_bounds = draw_bounds;
    draw_bounds.intersect(m_clip);

    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    auto color = fill_color();

    int alpha_diff = color.alpha() - shading.final_alpha();
    int step, skipped_steps, end_x;

    switch (shading.type()) {
    case TopToBottom:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = min_y - orig_bounds.min_y();
        color.set_alpha(color.alpha() - skipped_steps * step);

        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                m_bitmap[y][x].mix_with(color);
            }
            color.set_alpha(color.alpha() - step);
        }
        return;

    case BottomToTop:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = orig_bounds.max_y() - max_y;
        color.set_alpha(color.alpha() - skipped_steps * step);

        for (int y = max_y; y >= min_y; y--) {
            for (int x = min_x; x <= max_x; x++) {
                m_bitmap[y][x].mix_with(color);
            }
            color.set_alpha(color.alpha() - step);
        }
        return;

    case LeftToRight:
        step = alpha_diff / orig_bounds.width();
        skipped_steps = min_x - orig_bounds.min_x();
        color.set_alpha(color.alpha() - skipped_steps * step);

        for (int x = min_x; x <= max_x; x++) {
            for (int y = min_y; y <= max_y; y++) {
                m_bitmap[y][x].mix_with(color);
            }
            color.set_alpha(color.alpha() - step);
        }
        return;

    case RightToLeft:
        step = alpha_diff / orig_bounds.width();
        skipped_steps = orig_bounds.max_x() - max_x;
        color.set_alpha(color.alpha() - skipped_steps * step);

        for (int x = max_x; x >= min_x; x--) {
            for (int y = min_y; y <= max_y; y++) {
                m_bitmap[y][x].mix_with(color);
            }
            color.set_alpha(color.alpha() - step);
        }
        return;

    case Deg45:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = orig_bounds.max_y() - max_y + min_x - orig_bounds.min_x();
        if (skipped_steps >= orig_bounds.height()) {
            return;
        }

        color.set_alpha(color.alpha() - skipped_steps * step);
        end_x = std::min(min_x + (int)orig_bounds.height() - skipped_steps, max_x);

        for (int y = max_y; y >= min_y; y--) {
            auto cur_color = color;
            for (int x = min_x; x <= end_x; x++) {
                m_bitmap[y][x].mix_with(cur_color);
                cur_color.set_alpha(cur_color.alpha() - step);
            }
            end_x--;
            if (!end_x) {
                return;
            }

            color.set_alpha(color.alpha() - step);
        }
        return;

    case Deg315:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = min_y - orig_bounds.min_y() + min_x - orig_bounds.min_x();
        if (skipped_steps >= orig_bounds.height()) {
            return;
        }

        color.set_alpha(color.alpha() - skipped_steps * step);
        end_x = std::min(min_x + (int)orig_bounds.height() - skipped_steps, max_x);

        for (int y = min_y; y <= max_y; y++) {
            auto cur_color = color;
            for (int x = min_x; x <= end_x; x++) {
                m_bitmap[y][x].mix_with(cur_color);
                cur_color.set_alpha(cur_color.alpha() - step);
            }
            end_x--;
            if (!end_x) {
                return;
            }

            color.set_alpha(color.alpha() - step);
        }
        return;

    case Deg135:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = orig_bounds.max_y() - max_y + orig_bounds.max_x() - max_x;
        if (skipped_steps >= orig_bounds.height()) {
            return;
        }

        color.set_alpha(color.alpha() - skipped_steps * step);
        end_x = std::max(max_x - ((int)orig_bounds.height() - skipped_steps), min_x);

        for (int y = max_y; y >= min_y; y--) {
            auto cur_color = color;
            for (int x = max_x; x >= end_x; x--) {
                m_bitmap[y][x].mix_with(cur_color);
                cur_color.set_alpha(cur_color.alpha() - step);
            }
            end_x++;
            if (end_x == max_x) {
                return;
            }

            color.set_alpha(color.alpha() - step);
        }
        return;

    case Deg225:
        step = alpha_diff / orig_bounds.height();
        skipped_steps = min_y - orig_bounds.min_y() + orig_bounds.max_x() - max_x;
        if (skipped_steps >= orig_bounds.height()) {
            return;
        }

        color.set_alpha(color.alpha() - skipped_steps * step);
        end_x = std::max(max_x - ((int)orig_bounds.height() - skipped_steps), min_x);

        for (int y = min_y; y <= max_y; y++) {
            auto cur_color = color;
            for (int x = max_x; x >= end_x; x--) {
                m_bitmap[y][x].mix_with(cur_color);
                cur_color.set_alpha(cur_color.alpha() - step);
            }
            end_x++;
            if (end_x == max_x) {
                return;
            }

            color.set_alpha(color.alpha() - step);
        }
        return;

    default:
        break;
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