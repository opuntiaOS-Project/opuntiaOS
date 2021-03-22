/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Screen.h"
#include "Compositor.h"
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <utility>

static Screen* s_the;

Screen& Screen::the()
{
    return *s_the;
}

Screen::Screen()
    : m_bounds(0, 0, 1024, 768)
    , m_depth(4)
    , m_write_bitmap()
    , m_display_bitmap()
{
    s_the = this;
    m_screen_fd = open("/dev/bga", O_RDWR);

    size_t screen_buffer_size = width() * height() * depth();
    auto* first_buffer = (LG::Color*)mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, m_screen_fd, 0);
    auto* second_buffer = (LG::Color*)((uint8_t*)first_buffer + screen_buffer_size);

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
