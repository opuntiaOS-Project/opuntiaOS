/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LoadingScreen.h"
#include "../Helpers/TextDrawer.h"
#include <libfoundation/Memory.h>
#include <libg/Context.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <unistd.h>

namespace WinServer {

LoadingScreen* s_WinServer_LoadingScreen_the = nullptr;

LoadingScreen::LoadingScreen()
    : m_screen(Screen::the())
{
    s_WinServer_LoadingScreen_the = this;
    run_intro_animation();
}

void LoadingScreen::display_status_bar(int progress, int out_of)
{
    LG::Context ctx(m_screen.write_bitmap());
    int widthp = (progress * progress_line_width()) / out_of;

    ctx.set_fill_color(LG::Color(20, 20, 20));
    ctx.fill_rounded(LG::Rect(m_progress_line_min_x, m_progress_line_min_y, progress_line_width(), progress_line_height()), LG::CornerMask(4));

    ctx.set_fill_color(LG::Color::White);
    ctx.fill_rounded(LG::Rect(m_progress_line_min_x, m_progress_line_min_y, widthp, progress_line_height()), LG::CornerMask(4));

    m_screen.swap_buffers();
}

void LoadingScreen::animation_frame(LG::Context& ctx, LG::Point<int> pt, int cur_frame, int total_frames)
{
    // Applying a cubic-easing.
    int animation_window = total_frames / m_logo_text.size();
    int animation_frames_per_char = 2 * animation_window;
    int frame = total_frames - cur_frame;
    frame = total_frames - frame * frame * frame / (total_frames * total_frames);

    for (int i = 0; i < m_logo_text.size(); i++) {
        int end_frame = animation_window * (i + 1);
        int start_frame = std::max(0, end_frame - animation_frames_per_char);

        // Knowing animation bounds for current char and current frame, we calculate of animation
        // at this frame. This data is used to calcualte alpha and offset for the current char.
        int animation_progress = std::max(0, std::min(frame - start_frame, animation_frames_per_char));
        int alpha = (255 * (animation_progress)) / animation_frames_per_char;

        // Offset is calculated based on alpha, maybe we could move out this consts.
        pt.offset_by((alpha / 60) - 4, 0);

        ctx.set_fill_color(LG::Color(255, 255, 255, alpha));
        ctx.draw(pt, m_font.glyph(m_logo_text[i]));
        pt.offset_by(m_font.glyph(m_logo_text[i]).advance(), 0);
    }
}

void LoadingScreen::run_intro_animation()
{
    const int frames = 102; // 1.7s * 60fps
    size_t text_height = 64;
    size_t text_width = Helpers::text_width(m_logo_text, m_font);

    // Preparing area for animation
    int animation_spread = 10;
    int content_min_x = m_screen.bounds().mid_x() - (text_width / 2);
    int content_min_y = m_screen.bounds().mid_y() - ((text_height + progress_line_height()) / 2);
    auto animation_rect = LG::Rect({ content_min_x - animation_spread, content_min_y, text_width + 2 * animation_spread, text_height });
    LG::Context ctx(m_screen.display_bitmap());

    for (int frame = 0; frame < frames; frame++) {
        ctx.set_fill_color(LG::Color::Black);
        ctx.fill(animation_rect);
        animation_frame(ctx, { content_min_x, content_min_y }, frame, frames);
        m_screen.swap_buffers();

        // Copying current progress to backing storage.
        auto* buf1_ptr = reinterpret_cast<uint32_t*>(&m_screen.display_bitmap()[content_min_y][content_min_x]);
        auto* buf2_ptr = reinterpret_cast<uint32_t*>(&m_screen.write_bitmap()[content_min_y][content_min_x]);
        for (int j = 0; j < text_height; j++) {
            LFoundation::fast_copy(buf2_ptr, buf1_ptr, text_width);
            buf1_ptr += m_screen.width();
            buf2_ptr += m_screen.width();
        }

        // Going to sleep until the next frame.
        usleep(900000 / 60);
    }

    m_progress_line_min_x = m_screen.bounds().mid_x() - (progress_line_width() / 2);
    m_progress_line_min_y = content_min_y + text_height;
}

} // namespace WinServer
