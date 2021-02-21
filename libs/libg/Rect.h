/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "Point.h"
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <std/Utility.h>
#include <sys/types.h>

namespace LG {

class Rect : public Encodable<Rect>, public Decodable<Rect> {
public:
    Rect() = default;
    Rect(int x, int y, size_t width, size_t height);

    ~Rect()= default;

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }
    inline size_t square() const { return width() * height(); }
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
    inline void offset_by(int x, int y) { m_origin.offset_by(x, y); }
    inline void offset_by(const Point<int>& p) { m_origin.offset_by(p); }

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

}