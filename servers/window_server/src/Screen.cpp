/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Screen.h"
#include "Compositor.h"
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <utility>

namespace WinServer {

Screen* s_WinServer_Screen_the = nullptr;

Screen::Screen()
    : m_depth(4)
    , m_write_bitmap()
    , m_display_bitmap()
{
    s_WinServer_Screen_the = this;
    m_screen_fd = open("/dev/bga", O_RDWR);
    m_bounds = LG::Rect(0, 0, ioctl(m_screen_fd, BGA_GET_WIDTH, 0), ioctl(m_screen_fd, BGA_GET_HEIGHT, 0));

    size_t screen_buffer_size = width() * height() * depth();
    auto* first_buffer = reinterpret_cast<LG::Color*>(mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_screen_fd, 0));
    auto* second_buffer = reinterpret_cast<LG::Color*>(reinterpret_cast<uint8_t*>(first_buffer) + screen_buffer_size);

    m_display_bitmap = LG::PixelBitmap(first_buffer, width(), height());
    m_write_bitmap = LG::PixelBitmap(second_buffer, width(), height());

    m_display_bitmap_ptr = &m_display_bitmap;
    m_write_bitmap_ptr = &m_write_bitmap;

    m_active_buffer = 0;
}

void Screen::swap_buffers()
{
    m_write_bitmap_ptr.swap(m_display_bitmap_ptr);
    m_active_buffer ^= 1;
    ioctl(m_screen_fd, BGA_SWAP_BUFFERS, m_active_buffer);
}

} // namespace WinServer
