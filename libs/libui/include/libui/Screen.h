/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <cstring>
#include <fcntl.h>
#include <libg/Rect.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <utility>

namespace UI {

class Screen;
static Screen* __main_screen;

class Screen {
public:
    Screen()
        : m_screen_fd(open("/dev/bga", O_RDONLY))
        , m_bounds(0, 0, ioctl(m_screen_fd, BGA_GET_WIDTH, 0), ioctl(m_screen_fd, BGA_GET_HEIGHT, 0))
    {
    }

    explicit Screen(const std::string& path)
        : m_screen_fd(open(path.c_str(), O_RDONLY))
        , m_bounds(0, 0, ioctl(m_screen_fd, BGA_GET_WIDTH, 0), ioctl(m_screen_fd, BGA_GET_HEIGHT, 0))
    {
    }

    ~Screen() = default;

    int scale() const { return 1; }
    const LG::Rect& bounds() const { return m_bounds; }

    static Screen& main()
    {
        if (!__main_screen) {
            __main_screen = new UI::Screen();
        }
        return *__main_screen;
    }

private:
    int m_screen_fd;
    LG::Rect m_bounds;
};

} // namespace UI