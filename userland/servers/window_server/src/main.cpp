/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Managers/InitManager.h"

#ifdef TARGET_DESKTOP
#define LAUNCH_PATH "/System/dock"
#elif TARGET_MOBILE
#define LAUNCH_PATH "/System/homescreen"
#endif

int main()
{
    WinServer::InitManager::load_screen();

    auto* event_loop = new LFoundation::EventLoop();
    WinServer::InitManager::load_core_component<WinServer::Connection>();
    WinServer::InitManager::load_core_component<WinServer::CursorManager>();
    WinServer::InitManager::load_core_component<WinServer::ResourceManager, 4>();
    Logger::debug << "here1" << std::endl;
    Logger::debug << "s_WinServer_Popup_the" << std::endl;
    WinServer::InitManager::load_core_component<WinServer::Popup>();
    Logger::debug << "here2" << std::endl;
    WinServer::InitManager::load_core_component<WinServer::MenuBar>();
#ifdef TARGET_MOBILE
    WinServer::InitManager::load_core_component<WinServer::ControlBar>();
#endif
Logger::debug << "here3" << std::endl;
    WinServer::InitManager::load_core_component<WinServer::Compositor>();
    Logger::debug << "here4" << std::endl;
    WinServer::InitManager::load_core_component<WinServer::WindowManager>();
    Logger::debug << "here5" << std::endl;
    WinServer::InitManager::load_core_component<WinServer::Devices>();

    WinServer::InitManager::add_widget<WinServer::Clock>();
    WinServer::InitManager::add_widget<WinServer::ControlPanelToggle>();

    WinServer::InitManager::launch_app(LAUNCH_PATH, 10);

    WinServer::LoadingScreen::destroy_the();
    return event_loop->run();
}