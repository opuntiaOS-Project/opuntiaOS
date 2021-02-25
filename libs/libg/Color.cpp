/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/Color.h>

namespace LG {

Color::Color(Colors clr)
{
    struct rgb {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb_color;

    switch (clr) {
    case Red:
        rgb_color = { 255, 0, 0 };
        break;
    case Blue:
        rgb_color = { 0, 0, 255 };
        break;
    case Green:
        rgb_color = { 0, 255, 0 };
        break;
    case White:
        rgb_color = { 255, 255, 255 };
        break;
    case Black:
        rgb_color = { 0, 0, 0 };
        break;
    case LightSystemBackground:
        rgb_color = { 249, 249, 249 };
        break;
    case LightSystemText:
        rgb_color = { 49, 49, 43 };
        break;
    case LightSystemButton:
        rgb_color = { 235, 235, 235 };
        break;
    case LightSystemAccentButton:
        rgb_color = { 62, 119, 233 };
        break;
    default:
        break;
    }

    m_r = rgb_color.r;
    m_g = rgb_color.g;
    m_b = rgb_color.b;
    m_opacity = 0;
}

}