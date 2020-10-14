/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Color.h"

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
    default:
        break;
    }

    m_r = rgb_color.r; 
    m_g = rgb_color.g;
    m_b = rgb_color.b;
    m_opacity = 0;
}


}