/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../IPC/Event.h"
#include "../Managers/WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <memory>

namespace WinServer {

class Devices {
public:
    inline static Devices& the()
    {
        extern Devices* s_WinServer_Devices_the;
        return *s_WinServer_Devices_the;
    }

    Devices();
    ~Devices() = default;

    inline void pump_mouse() const
    {
        LFoundation::EventLoop& el = LFoundation::EventLoop::the();
        WindowManager& wm = WindowManager::the();

        char buf[512];
        int read_cnt = read(m_mouse_fd, buf, sizeof(buf));
        if (read_cnt <= 0) {
            return;
        }

        auto* packet_buf = reinterpret_cast<MousePacket*>(buf);
        for (int offset = 0, cnt = 0; offset < read_cnt; offset += sizeof(MousePacket), cnt++) {
            el.add(wm, new MouseEvent(packet_buf[cnt]));
        }
    }

    inline void pump_keyboard() const
    {
        LFoundation::EventLoop& el = LFoundation::EventLoop::the();
        WindowManager& wm = WindowManager::the();

        char buf[512];
        int read_cnt = read(m_keyboard_fd, buf, sizeof(buf));
        if (read_cnt <= 0) {
            return;
        }

        auto* packet_buf = reinterpret_cast<KeyboardPacket*>(buf);
        for (int offset = 0, cnt = 0; offset < read_cnt; offset += sizeof(KeyboardPacket), cnt++) {
            el.add(wm, new KeyboardEvent(packet_buf[cnt]));
        }
    }

private:
    int m_mouse_fd;
    int m_keyboard_fd;
};

} // namespace WinServer