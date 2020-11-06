/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSEvent.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <memory.h>
#include <syscalls.h>

class Devices {
public:
    static Devices& the();
    Devices();
    ~Devices() {}

    inline void pump_mouse()
    {
        LFoundation::EventLoop& el = LFoundation::EventLoop::the();
        WindowManager& wm = WindowManager::the();

        char buf[512];
        int read_cnt = read(m_mouse_fd, buf, sizeof(buf));
        if (read_cnt <= 0) {
            return;
        }

        MousePacket* packet_buf = (MousePacket*)buf;
        for (int offset = 0, cnt = 0; offset < read_cnt; offset += sizeof(MousePacket), cnt++) {
            el.add(wm, new MouseEvent(packet_buf[cnt]));
        }
    }

    inline void pump_keyboard()
    {
        LFoundation::EventLoop& el = LFoundation::EventLoop::the();
        WindowManager& wm = WindowManager::the();

        char buf[512];
        int read_cnt = read(m_keyboard_fd, buf, sizeof(buf));
        if (read_cnt <= 0) {
            return;
        }

        KeyboardPacket* packet_buf = (KeyboardPacket*)buf;
        for (int offset = 0, cnt = 0; offset < read_cnt; offset += sizeof(KeyboardPacket), cnt++) {
            el.add(wm, new KeyboardEvent(packet_buf[cnt]));
        }
    }

private:
    int m_mouse_fd;
    int m_keyboard_fd;
};