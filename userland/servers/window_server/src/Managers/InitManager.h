/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../Components/ControlBar/ControlBar.h"
#include "../Components/LoadingScreen/LoadingScreen.h"
#include "../Components/MenuBar/MenuBar.h"
#include "../Components/MenuBar/Widgets/Clock/Clock.h"
#include "../Components/MenuBar/Widgets/ControlPanelToggle/ControlPanelToggle.h"
#include "../Components/Popup/Popup.h"
#include "../Devices/Devices.h"
#include "../Devices/Screen.h"
#include "../IPC/Connection.h"
#include "Compositor.h"
#include "CursorManager.h"
#include "ResourceManager.h"
#include "WindowManager.h"
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <new>
#include <sys/socket.h>
#include <unistd.h>

namespace WinServer {

class InitManager {
public:
    InitManager() = delete;
    ~InitManager() = delete;

    static void load_screen()
    {
        nice(-3);
        new WinServer::Screen();
        new WinServer::LoadingScreen();
    }

    template <class T, int Cost = 1, class... Args>
    static constexpr inline void load_core_component(Args&&... args)
    {
        Logger::debug << "allocing " << std::endl;
        new T(std::forward<Args>(args)...);
        Logger::debug << "going new" << std::endl;
        WinServer::LoadingScreen::the().move_progress<T, Cost>();
    }

    template <class T, int Cost = 1, class... Args>
    static constexpr inline void add_widget(Args&&... args)
    {
        WinServer::MenuBar::the().add_widget<T>(std::forward<Args>(args)...);
        WinServer::LoadingScreen::the().move_progress<T, Cost>();
    }

    static inline int launch_app(const char* path, int uid = 0)
    {
        int pid = fork();
        if (!pid) {
            setuid(uid);
            setgid(uid);
            for (int i = 3; i < 32; i++) {
                close(i);
            }

            execlp(path, path, NULL);
            std::abort();
        }
        return pid;
    }

private:
};

};