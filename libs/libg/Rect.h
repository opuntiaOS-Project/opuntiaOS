#pragma once

#include <sys/types.h>

namespace LG {

class Rect {
public:
    Rect(int x, int y, size_t width, size_t height);
    ~Rect() { }

    inline int x() const { return m_x; }
    inline int y() const { return m_y; }
    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }

    inline bool contains(int x, int y) const { return m_x <= x && x <= m_x + m_width && m_y <= y && y <= m_y + m_height; }
    inline void offset_by(int x, int y) { m_x += x, m_y += y; }

private:
    int m_x;
    int m_y;
    size_t m_width;
    size_t m_height;
};

}