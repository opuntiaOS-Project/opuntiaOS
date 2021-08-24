/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "DockView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

class DockViewController : public UI::ViewController<DockView> {
public:
    DockViewController(DockView& view)
        : UI::ViewController<DockView>(view)
    {
    }
    virtual ~DockViewController() = default;

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color(255, 255, 255, 135));
        view().new_fast_launch_entity("/res/icons/apps/about.icon", "/Applications/about.app/Content/about"); // FIXME: Parse some init file
        view().new_fast_launch_entity("/res/icons/apps/terminal.icon", "/Applications/terminal.app/Content/terminal");
        view().new_fast_launch_entity("/res/icons/apps/activity_monitor.icon", "/Applications/activity_monitor.app/Content/activity_monitor");
        view().new_fast_launch_entity("/res/icons/apps/calculator.icon", "/Applications/calculator.app/Content/calculator");
        view().set_needs_display();
    }

private:
};