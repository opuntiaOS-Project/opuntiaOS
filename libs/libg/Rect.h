/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <std/Utility.h>
#include <sys/types.h>

namespace LG {

class Rect {
public:
    inline Rect() = default;
    inline Rect(int x, int y, size_t width, size_t height)
        : m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
    {
    }

    ~Rect() { }

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }
    inline int min_x() const { return m_x; }
    inline int mid_x() const { return m_x + width() / 2; }
    inline int max_x() const { return m_x + width() - 1; }
    inline int min_y() const { return m_y; }
    inline int mid_y() const { return m_y + height() / 2; }
    inline int max_y() const { return m_y + height() - 1; }
    inline bool empty() const { return !width() || !height(); }

    inline void set_x(int new_x) { m_x = new_x; }
    inline void set_y(int new_y) { m_y = new_y; }
    inline void set_width(int new_w) { m_width = new_w; }
    inline void set_height(int new_h) { m_height = new_h; }

    inline bool contains(int x, int y) const { return m_x <= x && x <= m_x + m_width && m_y <= y && y <= m_y + m_height; }
    inline void offset_by(int x, int y) { m_x += x, m_y += y; }

    void intersect(const Rect& other);
private:
    int m_x { 0 };
    int m_y { 0 };
    size_t m_width { 0 };
    size_t m_height { 0 };
};

}