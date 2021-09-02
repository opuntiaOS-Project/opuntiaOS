/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <algorithm>
#include <libg/Point.h>
#include <libg/Size.h>
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <sys/types.h>
#include <utility>

namespace LG {

class Rect : public Encodable<Rect>, public Decodable<Rect> {
public:
    Rect() = default;
    Rect(int x, int y, size_t width, size_t height);

    ~Rect() = default;

    Rect& operator=(const Rect& r)
    {
        m_origin = r.origin(), m_width = r.width(), m_height = r.height();
        return *this;
    }

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }
    inline size_t square() const { return width() * height(); }
    inline LG::Size size() const { return LG::Size(m_width, m_height); }
    inline int min_x() const { return m_origin.x(); }
    inline int mid_x() const { return m_origin.x() + width() / 2; }
    inline int max_x() const { return m_origin.x() + width() - 1; }
    inline int min_y() const { return m_origin.y(); }
    inline int mid_y() const { return m_origin.y() + height() / 2; }
    inline int max_y() const { return m_origin.y() + height() - 1; }
    inline bool empty() const { return !width() || !height(); }

    inline void set_x(int new_x) { m_origin.set_x(new_x); }
    inline void set_y(int new_y) { m_origin.set_y(new_y); }
    inline void set_width(int new_w) { m_width = new_w; }
    inline void set_height(int new_h) { m_height = new_h; }

    inline bool contains(int x, int y) const { return min_x() <= x && x <= max_x() && min_y() <= y && y <= max_y(); }
    inline bool contains(const Point<int>& p) const { return contains(p.x(), p.y()); }
    inline bool contains(const Rect& p) const { return contains(p.min_x(), p.min_y()) && contains(p.max_x(), p.max_y()); }
    inline void offset_by(int x, int y) { m_origin.offset_by(x, y); }
    inline void offset_by(const Point<int>& p) { m_origin.offset_by(p); }

    inline void set_origin(const Point<int>& origin) { m_origin = origin; }
    inline void set_origin(Point<int>&& origin) { m_origin = std::move(origin); }
    inline Point<int>& origin() { return m_origin; }
    inline const Point<int>& origin() const { return m_origin; }

    void unite(const Rect& other);
    LG::Rect union_of(const Rect& other) const;
    void intersect(const Rect& other);
    bool intersects(const Rect& other) const;
    LG::Rect intersection(const Rect& other) const;

    void encode(EncodedMessage& buf) const override;
    void decode(const char* buf, size_t& offset) override;

    bool operator==(const Rect& r) const
    {
        return m_width == r.m_width && m_height == r.m_height && m_origin == r.m_origin;
    }

    bool operator!=(const Rect& r) const
    {
        return !(*this == r);
    }

private:
    Point<int> m_origin;
    size_t m_width { 0 };
    size_t m_height { 0 };
};

// Implementation

inline void Rect::unite(const Rect& other)
{
    // (a, b) - top left corner
    // (c, d) - bottom right corner
    int a = std::min(min_x(), other.min_x());
    int b = std::min(min_y(), other.min_y());
    int c = std::max(max_x(), other.max_x());
    int d = std::max(max_y(), other.max_y());

    set_x(a);
    set_y(b);
    set_width(c - a + 1);
    set_height(d - b + 1);
}

inline LG::Rect Rect::union_of(const Rect& other) const
{
    // (a, b) - top left corner
    // (c, d) - bottom right corner
    int a = std::min(min_x(), other.min_x());
    int b = std::min(min_y(), other.min_y());
    int c = std::max(max_x(), other.max_x());
    int d = std::max(max_y(), other.max_y());
    return LG::Rect(a, b, c - a + 1, d - b + 1);
}

inline void Rect::intersect(const Rect& other)
{
    // (a, b) - top left corner
    // (c, d) - bottom right corner
    int a = std::max(min_x(), other.min_x());
    int b = std::max(min_y(), other.min_y());
    int c = std::min(max_x(), other.max_x());
    int d = std::min(max_y(), other.max_y());

    if (a > c || b > d) {
        set_width(0);
        set_height(0);
        return;
    }

    set_x(a);
    set_y(b);
    set_width(c - a + 1);
    set_height(d - b + 1);
}

inline bool Rect::intersects(const Rect& other) const
{
    // (a, b) - top left corner
    // (c, d) - bottom right corner
    int a = std::max(min_x(), other.min_x());
    int b = std::max(min_y(), other.min_y());
    int c = std::min(max_x(), other.max_x());
    int d = std::min(max_y(), other.max_y());
    return c >= a && d >= b;
}

inline LG::Rect Rect::intersection(const Rect& other) const
{
    // (a, b) - top left corner
    // (c, d) - bottom right corner
    int a = std::max(min_x(), other.min_x());
    int b = std::max(min_y(), other.min_y());
    int c = std::min(max_x(), other.max_x());
    int d = std::min(max_y(), other.max_y());

    if (a > c || b > d) {
        return LG::Rect(0, 0, 0, 0);
    }
    return LG::Rect(a, b, c - a + 1, d - b + 1);
}

} // namespace LG