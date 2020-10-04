/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <memory.h>
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();

    void swap_buffers();
    void run();

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }
    inline uint32_t depth() const { return m_depth; }

    inline LG::PixelBitmap& write_bitmap() { return *m_write_bitmap_ptr; }
    inline const LG::PixelBitmap& write_bitmap() const { return *m_write_bitmap_ptr; }
    inline LG::PixelBitmap& display_bitmap() { return *m_display_bitmap_ptr; }
    inline const LG::PixelBitmap& display_bitmap() const { return *m_display_bitmap_ptr; }

private:
    int m_screen_fd;
    size_t m_width;
    size_t m_height;
    uint32_t m_depth;

    int m_active_buffer;

    LG::PixelBitmap m_write_bitmap;
    LG::PixelBitmap m_display_bitmap;

    UniquePtr<LG::PixelBitmap> m_write_bitmap_ptr { nullptr };
    UniquePtr<LG::PixelBitmap> m_display_bitmap_ptr { nullptr };
};