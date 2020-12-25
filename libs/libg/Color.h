/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libcxx/sys/types.h>

namespace LG {

// We keep opacity as an opposite to alpha.
// This is used for capability with BGA driver
// and probably should be fixed.
class Color {
public:
    enum Colors {
        Red,
        Blue,
        Green,
        White,
        Black,
    };

    Color() = default;
    Color(Colors);
    Color(const Color& c)
        : m_opacity(c.m_opacity)
        , m_r(c.m_r)
        , m_g(c.m_g)
        , m_b(c.m_b)
    {
    }

    Color(uint32_t color)
        : m_opacity((color & 0xFF000000) >> 24)
        , m_r((color & 0x00FF0000) >> 16)
        , m_g((color & 0x0000FF00) >> 8)
        , m_b((color & 0x000000FF) >> 0)
    {
    }

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 255)
        : m_opacity(255 - alpha)
        , m_b(b)
        , m_g(g)
        , m_r(r)
    {
    }

    ~Color() { }

    Color& operator=(const Color& c)
    {
        m_opacity = c.m_opacity;
        m_r = c.m_r;
        m_g = c.m_g;
        m_b = c.m_b;
        return *this;
    }

    Color& operator=(Color&& c)
    {
        m_opacity = c.m_opacity;
        m_r = c.m_r;
        m_g = c.m_g;
        m_b = c.m_b;
        return *this;
    }

    inline uint8_t alpha() const { return 255 - m_opacity; }
    inline bool is_opaque() const { return m_opacity == 255; }

    inline uint8_t red() const { return m_r; }
    inline uint8_t green() const { return m_g; }
    inline uint8_t blue() const { return m_b; }

    inline uint32_t u32() const
    {
        uint32_t clr = (m_opacity << 24)
            | (m_b << 0)
            | (m_g << 8)
            | (m_r << 16);
        return clr;
    }

    [[gnu::always_inline]] inline void mix_with(const Color& clr)
    {
        if (clr.is_opaque() || !alpha()) {
            return;
        }

        int alpha_c = 256 * (alpha() + clr.alpha()) - alpha() * clr.alpha();
        int alpha_of_me = alpha() * (255 - clr.alpha());
        int alpha_of_it = 256 * clr.alpha();

        m_r = (red() * alpha_of_me + clr.red() * alpha_of_it) / alpha_c;
        m_g = (green() * alpha_of_me + clr.green() * alpha_of_it) / alpha_c;
        m_b = (blue() * alpha_of_me + clr.blue() * alpha_of_it) / alpha_c;
        m_opacity = 255 - (alpha_c / 256);
    }

private:
    uint8_t m_b { 0 };
    uint8_t m_g { 0 };
    uint8_t m_r { 0 };
    uint8_t m_opacity { 0 };
};

}