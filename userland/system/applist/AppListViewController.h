/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "AppListView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

class AppListViewController : public UI::ViewController<AppListView> {
public:
    AppListViewController(AppListView& view)
        : UI::ViewController<AppListView>(view)
    {
    }
    virtual ~AppListViewController() = default;

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemOpaque);
        view().layer().set_corner_mask(LG::CornerMask(4, LG::CornerMask::Masked, LG::CornerMask::NonMasked));
        view().new_dock_entity("/Applications/about.app/Content/about", "/res/icons/apps/about.icon", "com.opuntia.about");
        view().new_dock_entity("/Applications/terminal.app/Content/terminal", "/res/icons/apps/terminal.icon", "com.opuntia.terminal");
        view().new_dock_entity("/Applications/activity_monitor.app/Content/activity_monitor", "/res/icons/apps/activity_monitor.icon", "com.opuntia.activity_monitor");
        view().new_dock_entity("/Applications/calculator.app/Content/calculator", "/res/icons/apps/calculator.icon", "com.opuntia.calculator");
        view().set_needs_display();
    }

private:
};