/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <libfoundation/Math.h>
#include <libfoundation/Memory.h>
#include <libg/Context.h>

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
    auto r = rect;
    r.offset_by(m_draw_offset);
    m_clip.intersect(r);
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
    int offset_x = -start.x() - m_draw_offset.x() + m_bitmap_offset.x();
    int offset_y = -start.y() - m_draw_offset.y() + m_bitmap_offset.y();
    int bitmap_x = min_x + offset_x;
    int bitmap_y = min_y + offset_y;
    int len_x = max_x - min_x + 1;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        LFoundation::fast_copy((uint32_t*)&m_bitmap[y][min_x], (uint32_t*)&bitmap[bitmap_y][bitmap_x], len_x);
    }
}

void Context::set_with_bounds(const Rect& rect, const PixelBitmap& bitmap)
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
    int offset_x = -rect.min_x() - m_draw_offset.x();
    int offset_y = -rect.min_y() - m_draw_offset.y();
    int bitmap_x = min_x + offset_x + m_bitmap_offset.x();
    int bitmap_y = min_y + offset_y + m_bitmap_offset.y();
    int len_x = max_x - min_x + 1;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        LFoundation::fast_copy((uint32_t*)&m_bitmap[y][min_x], (uint32_t*)&bitmap[bitmap_y][bitmap_x], len_x);
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
    int offset_x = -start.x() - m_draw_offset.x() + m_bitmap_offset.x();
    int offset_y = -start.y() - m_draw_offset.y() + m_bitmap_offset.y();
    int bitmap_y = min_y + offset_y;
    for (int y = min_y; y <= max_y; y++, bitmap_y++) {
        int bitmap_x = min_x + offset_x;
        for (int x = min_x; x <= max_x; x++, bitmap_x++) {
            m_bitmap[y][x].mix_with(bitmap[bitmap_y][bitmap_x]);
        }
    }
}

void Context::draw_with_bounds(const Rect& rect, const PixelBitmap& bitmap)
{
    if (!bitmap.has_alpha_channel()) {
        set_with_bounds(rect, bitmap);
        return;
    }

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
    int offset_x = -rect.min_x() - m_draw_offset.x() + m_bitmap_offset.x();
    int offset_y = -rect.min_y() - m_draw_offset.y() + m_bitmap_offset.y();
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

[[gnu::flatten]] void Context::draw_rounded(const Point<int>& start, const PixelBitmap& bitmap, const CornerMask& mask)
{
    Rect rect(start.x(), start.y(), bitmap.width(), bitmap.height());
    auto draw_bounds = rect;
    draw_bounds.offset_by(m_draw_offset);
    draw_bounds.intersect(m_clip);
    if (draw_bounds.empty()) {
        return;
    }

    size_t radius = std::min(mask.radius(), rect.height() / 2);
    radius = std::min(radius, rect.width() / 2);

    size_t top_radius = radius;
    size_t bottom_radius = radius;
    if (!mask.top_rounded()) {
        top_radius = 0;
    }
    if (!mask.bottom_rounded()) {
        bottom_radius = 0;
    }

    int width = rect.width();
    int top_rwidth = rect.width() - 2 * top_radius;
    int bottom_rwidth = rect.width() - 2 * bottom_radius;
    int rheight = rect.height() - top_radius - bottom_radius;

    int min_x = rect.min_x();
    int min_y = rect.min_y();
    int top_min_rx = rect.min_x() + top_radius;
    int top_max_rx = rect.max_x() - top_radius;
    int bottom_min_rx = rect.min_x() + bottom_radius;
    int bottom_max_rx = rect.max_x() - bottom_radius;
    int min_ry = rect.min_y() + top_radius;
    int max_ry = rect.max_y() - bottom_radius;
    int bitmap_bottom_x_offset = rect.width() - bottom_radius;
    int bitmap_bottom_y_offset = rect.height() - bottom_radius;

    m_bitmap_offset = { (int)top_radius, 0 };
    draw_with_bounds(LG::Rect(top_min_rx, min_y, top_rwidth, top_radius), bitmap);
    m_bitmap_offset = { 0, (int)top_radius };
    draw_with_bounds(LG::Rect(min_x, min_ry, width, rheight), bitmap);
    m_bitmap_offset = { (int)bottom_radius, bitmap_bottom_y_offset };
    draw_with_bounds(LG::Rect(bottom_min_rx, max_ry + 1, bottom_rwidth, bottom_radius), bitmap);

    auto add_instant_clip = [&](int x, int y, size_t width, size_t height) {
        add_clip(LG::Rect(x + m_draw_offset.x(), y + m_draw_offset.y(), width, height));
    };

    auto orig_clip = m_clip;
    auto reset_instant_clip = [&]() {
        m_clip = orig_clip;
    };

    int bitmap_bottom_x_circle_offset = rect.width() - bottom_radius - bottom_radius - 1;
    int bitmap_bottom_y_circle_offset = rect.height() - bottom_radius - bottom_radius - 1;
    add_instant_clip(min_x, min_y, top_radius, top_radius);
    m_bitmap_offset = { 0, 0 };
    draw_rounded_helper({ top_min_rx, min_ry }, top_radius, bitmap);
    reset_instant_clip();

    add_instant_clip(top_max_rx + 1, min_y, top_radius, top_radius);
    m_bitmap_offset = { bitmap_bottom_x_circle_offset, 0 };
    draw_rounded_helper({ top_max_rx, min_ry }, top_radius, bitmap);
    reset_instant_clip();

    add_instant_clip(min_x, max_ry + 1, bottom_radius, bottom_radius);
    m_bitmap_offset = { 0, bitmap_bottom_y_circle_offset };
    draw_rounded_helper({ bottom_min_rx, max_ry }, bottom_radius, bitmap);
    reset_instant_clip();

    add_instant_clip(bottom_max_rx + 1, max_ry + 1, bottom_radius, bottom_radius);
    m_bitmap_offset = { bitmap_bottom_x_circle_offset, bitmap_bottom_y_circle_offset };
    draw_rounded_helper({ bottom_max_rx, max_ry }, bottom_radius, bitmap);
    reset_instant_clip();
    m_bitmap_offset = { 0, 0 };
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
        LFoundation::fast_set((uint32_t*)&m_bitmap[y][min_x], color, len_x);
    }
}

void Context::draw_rounded_helper(const Point<int>& start, size_t radius, const PixelBitmap& bitmap)
{
    if (!radius) {
        return;
    }

    auto center = start;
    center.offset_by(m_draw_offset);
    auto draw_bounds = Rect(center.x() - radius, center.y() - radius, radius * 2 + 1, radius * 2 + 1);
    draw_bounds.intersect(m_clip);

    if (draw_bounds.empty()) {
        return;
    }

    int radius2 = radius * radius;
    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int offset_x = -(start.x() - radius) - m_draw_offset.x() + m_bitmap_offset.x();
    int offset_y = -(start.y() - radius) - m_draw_offset.y() + m_bitmap_offset.y();
    int bitmap_y = min_y + offset_y;

    for (int y = min_y; y <= max_y; y++) {
        int bitmap_x = min_x + offset_x;
        for (int x = min_x; x <= max_x; x++) {
            int x2 = (x - center.x()) * (x - center.x());
            int y2 = (y - center.y()) * (y - center.y());
            int dist = x2 + y2;
            if (dist <= radius2) {
                m_bitmap[y][x].mix_with(bitmap[bitmap_y][bitmap_x]);
            } else {
                auto color = bitmap[bitmap_y][bitmap_x];
                float fdist = 0.5 - (LFoundation::fast_sqrt((float)(dist)) - radius);
                fdist = std::max(std::min(fdist, 1.0f), 0.0f);
                int alpha = int(color.alpha() * fdist);
                color.set_alpha(alpha);
                m_bitmap[y][x].mix_with(color);
            }
        }
    }
}

void Context::fill_rounded_helper(const Point<int>& start, size_t radius)
{
    if (!radius) {
        return;
    }

    auto center = start;
    center.offset_by(m_draw_offset);
    auto draw_bounds = Rect(center.x() - radius, center.y() - radius, radius * 2 + 1, radius * 2 + 1);
    draw_bounds.intersect(m_clip);

    if (draw_bounds.empty()) {
        return;
    }

    auto color = fill_color();
    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int radius2 = radius * radius;
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int x2 = (x - center.x()) * (x - center.x());
            int y2 = (y - center.y()) * (y - center.y());
            int dist = x2 + y2;
            if (dist <= radius2) {
                m_bitmap[y][x] = fill_color();
            } else {
                float fdist = 0.5 - (LFoundation::fast_sqrt((float)(dist)) - radius);
                fdist = std::max(std::min(fdist, 1.0f), 0.0f);
                int alpha = int(255.0 * fdist);
                color.set_alpha(alpha);
                m_bitmap[y][x].mix_with(color);
            }
        }
    }
}

void Context::shadow_rounded_helper(const Point<int>& start, size_t radius, const Shading& shading)
{
    if (!radius) {
        return;
    }

    auto center = start;
    center.offset_by(m_draw_offset);
    uint32_t hw = shading.spread() * 2 + radius * 2 + 1;
    auto draw_bounds = Rect(center.x() - radius - shading.spread(), center.y() - radius - shading.spread(), hw, hw);
    draw_bounds.intersect(m_clip);

    if (draw_bounds.empty()) {
        return;
    }

    auto color = fill_color();
    float shading_spread = shading.spread();
    float std_alpha = color.alpha();
    std_alpha /= (shading_spread - 1);
    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int radius2 = radius * radius;
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            int x2 = (x - center.x()) * (x - center.x());
            int y2 = (y - center.y()) * (y - center.y());
            int dist = x2 + y2;
            if (dist > radius2) {
                float fdist = LFoundation::fast_sqrt((float)(dist)) - radius;
                if (fdist > 0.1) {
                    fdist = shading_spread - fdist;
                    fdist = std::max(fdist, 0.0f);
                    int alpha = std_alpha * fdist;
                    color.set_alpha(alpha);
                    m_bitmap[y][x].mix_with(color);
                }
            }
        }
    }
}

[[gnu::flatten]] void Context::fill_rounded(const Rect& rect, const CornerMask& mask)
{
    auto draw_bounds = rect;
    draw_bounds.offset_by(m_draw_offset);
    draw_bounds.intersect(m_clip);
    if (draw_bounds.empty()) {
        return;
    }

    size_t radius = std::min(mask.radius(), rect.height() / 2);
    radius = std::min(radius, rect.width() / 2);

    size_t top_radius = radius;
    size_t bottom_radius = radius;
    if (!mask.top_rounded()) {
        top_radius = 0;
    }
    if (!mask.bottom_rounded()) {
        bottom_radius = 0;
    }

    int width = rect.width();
    int top_rwidth = rect.width() - 2 * top_radius;
    int bottom_rwidth = rect.width() - 2 * bottom_radius;
    int rheight = rect.height() - top_radius - bottom_radius;

    int min_x = rect.min_x();
    int min_y = rect.min_y();
    int top_min_rx = rect.min_x() + top_radius;
    int top_max_rx = rect.max_x() - top_radius;
    int bottom_min_rx = rect.min_x() + bottom_radius;
    int bottom_max_rx = rect.max_x() - bottom_radius;
    int min_ry = rect.min_y() + top_radius;
    int max_ry = rect.max_y() - bottom_radius;

    fill(LG::Rect(top_min_rx, min_y, top_rwidth, top_radius));
    fill(LG::Rect(min_x, min_ry, width, rheight));
    fill(LG::Rect(bottom_min_rx, max_ry + 1, bottom_rwidth, bottom_radius));

    auto add_instant_clip = [&](int x, int y, size_t width, size_t height) {
        add_clip(LG::Rect(x + m_draw_offset.x(), y + m_draw_offset.y(), width, height));
    };

    auto orig_clip = m_clip;
    auto reset_instant_clip = [&]() {
        m_clip = orig_clip;
    };

    add_instant_clip(min_x, min_y, top_radius, top_radius);
    fill_rounded_helper({ top_min_rx, min_ry }, top_radius);
    reset_instant_clip();

    add_instant_clip(top_max_rx + 1, min_y, top_radius, top_radius);
    fill_rounded_helper({ top_max_rx, min_ry }, top_radius);
    reset_instant_clip();

    add_instant_clip(min_x, max_ry + 1, bottom_radius, bottom_radius);
    fill_rounded_helper({ bottom_min_rx, max_ry }, bottom_radius);
    reset_instant_clip();

    add_instant_clip(bottom_max_rx + 1, max_ry + 1, bottom_radius, bottom_radius);
    fill_rounded_helper({ bottom_max_rx, max_ry }, bottom_radius);
    reset_instant_clip();
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
    case Shading::Type::TopToBottom:
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

    case Shading::Type::BottomToTop:
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

    case Shading::Type::LeftToRight:
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

    case Shading::Type::RightToLeft:
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

    case Shading::Type::Deg45:
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

    case Shading::Type::Deg315:
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

    case Shading::Type::Deg135:
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

    case Shading::Type::Deg225:
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

void Context::draw_box_shading(const Rect& rect, const Shading& shading, const CornerMask& mask)
{
    size_t top_radius = mask.radius();
    size_t bottom_radius = mask.radius();
    if (!mask.top_rounded()) {
        top_radius = 0;
    }
    if (!mask.bottom_rounded()) {
        bottom_radius = 0;
    }

    int shading_spread = shading.spread();

    int rwidth = rect.width() - 2 * top_radius;
    int rheight = rect.height() - top_radius - bottom_radius;

    int top_min_rx = rect.min_x() + top_radius;
    int top_min_ry = rect.min_y() + top_radius;
    int bottom_min_rx = rect.min_x() + bottom_radius;
    int bottom_max_ry = rect.max_y() - bottom_radius;
    int top_max_rx = rect.max_x() - top_radius;
    int bottom_max_rx = rect.max_x() - bottom_radius;
    int min_shading_x = rect.min_x() - shading_spread;
    int max_shading_x = rect.max_x();
    int min_shading_y = rect.min_y() - shading_spread;
    int max_shading_y = rect.max_y();

    draw_shading(LG::Rect(top_min_rx, min_shading_y, rwidth, shading_spread), LG::Shading(LG::Shading::Type::BottomToTop, shading.final_alpha()));
    draw_shading(LG::Rect(top_min_rx, max_shading_y, rwidth, shading_spread), LG::Shading(LG::Shading::Type::TopToBottom, shading.final_alpha()));
    draw_shading(LG::Rect(min_shading_x, top_min_ry, shading_spread, rheight), LG::Shading(LG::Shading::Type::RightToLeft, shading.final_alpha()));
    draw_shading(LG::Rect(max_shading_x, top_min_ry, shading_spread, rheight), LG::Shading(LG::Shading::Type::LeftToRight, shading.final_alpha()));

    auto add_instant_clip = [&](int x, int y, size_t width, size_t height) {
        add_clip(LG::Rect(x + m_draw_offset.x(), y + m_draw_offset.y(), width, height));
    };

    auto orig_clip = m_clip;
    auto reset_instant_clip = [&]() {
        m_clip = orig_clip;
    };

    int top_shading_dims = top_radius + shading_spread;
    int bottom_shading_dims = bottom_radius + shading_spread;

    if (mask.top_rounded()) {
        add_instant_clip(min_shading_x, min_shading_y, top_shading_dims, top_shading_dims);
        shadow_rounded_helper({ top_min_rx, top_min_ry }, top_radius, shading);
        reset_instant_clip();

        add_instant_clip(top_max_rx, min_shading_y, top_shading_dims, top_shading_dims);
        shadow_rounded_helper({ top_max_rx, top_min_ry }, top_radius, shading);
        reset_instant_clip();
    } else {
        draw_shading(LG::Rect(min_shading_x, min_shading_y, shading_spread, shading_spread), LG::Shading(LG::Shading::Type::Deg135, 0));
        draw_shading(LG::Rect(top_max_rx, min_shading_y, shading_spread, shading_spread), LG::Shading(LG::Shading::Type::Deg45, 0));
    }

    if (mask.bottom_rounded()) {
        add_instant_clip(min_shading_x, bottom_max_ry, bottom_shading_dims, bottom_shading_dims);
        shadow_rounded_helper({ bottom_min_rx, bottom_max_ry }, bottom_radius, shading);
        reset_instant_clip();

        add_instant_clip(bottom_max_rx, bottom_max_ry, bottom_shading_dims, bottom_shading_dims);
        shadow_rounded_helper({ bottom_max_rx, bottom_max_ry }, bottom_radius, shading);
        reset_instant_clip();
    } else {
        draw_shading(LG::Rect(min_shading_x, bottom_max_ry, shading_spread, shading_spread), LG::Shading(LG::Shading::Type::Deg225, 0));
        draw_shading(LG::Rect(bottom_max_rx, bottom_max_ry, shading_spread, shading_spread), LG::Shading(LG::Shading::Type::Deg315, 0));
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

} // namespace LG