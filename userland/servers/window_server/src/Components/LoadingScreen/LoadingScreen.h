/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../Devices/Screen.h"
#include <libg/Color.h>
#include <libg/Context.h>
#include <libg/Font.h>
#include <libg/PixelBitmap.h>
#include <memory>
#include <string_view>

namespace WinServer {

template <typename T, int Cost = 1>
class AfterComponentLoadProgress {
public:
    static const int cost = Cost;
    static const size_t progress;
};

static size_t total_cost = 0;
template <int Cost>
inline size_t __calc_total_cost()
{
    return total_cost += Cost;
}

template <typename T, int Cost>
const size_t AfterComponentLoadProgress<T, Cost>::progress = __calc_total_cost<Cost>();

class LoadingScreen {
public:
    inline static LoadingScreen& the()
    {
        extern LoadingScreen* s_WinServer_LoadingScreen_the;
        return *s_WinServer_LoadingScreen_the;
    }

    inline static void destroy_the()
    {
        extern LoadingScreen* s_WinServer_LoadingScreen_the;
        delete s_WinServer_LoadingScreen_the;
        s_WinServer_LoadingScreen_the = nullptr;
    }

    LoadingScreen();
    ~LoadingScreen() = default;

    template <typename T, int cost = 1>
    inline void move_progress() { display_status_bar(AfterComponentLoadProgress<T, cost>::progress, total_cost); }

private:
    static constexpr int progress_line_height() { return 4; }
    static constexpr int progress_line_width() { return 128; }

    void display_status_bar(int current_progress, int max_progress);

    void animation_frame(LG::Context& ctx, LG::Point<int> pt, int frame, int total_frames);
    void run_intro_animation();

    std::string_view m_logo_text { "opuntiaOS" };

    Screen& m_screen;
    int m_progress_line_min_x { 0 };
    int m_progress_line_min_y { 0 };
    LG::PixelBitmap m_logo;

    LG::Font& m_font { LG::Font::system_bold_font(32) };
};

} // namespace WinServer