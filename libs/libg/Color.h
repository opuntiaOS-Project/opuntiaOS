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

class Color {
public:
    Color(uint32_t color)
        : m_opacity((color & 0xFF000000) >> 24)
        , m_b((color & 0x00FF0000) >> 16)
        , m_g((color & 0x0000FF00) >> 8)
        , m_r((color & 0x000000FF) >> 0)
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

private:
    uint8_t m_r;
    uint8_t m_g;
    uint8_t m_b;
    uint8_t m_opacity;
};

}