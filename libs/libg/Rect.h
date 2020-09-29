#pragma once

namespace LG {

class Rect {
public:
    Rect(int x, int y, int width, int height);
    ~Rect() { }

    int x() const { return m_x; }
    int y() const { return m_y; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
};

}