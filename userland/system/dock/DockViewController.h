/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
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
#include <libfoundation/json/Parser.h>

class DockViewController : public UI::ViewController<DockView> {
public:
    DockViewController(DockView& view)
        : UI::ViewController<DockView>(view)
    {
        dock_apps.push_back("about");
        dock_apps.push_back("terminal");
        dock_apps.push_back("calculator");
        dock_apps.push_back("activity_monitor");
    }
    virtual ~DockViewController() = default;

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemOpaque);
        for (auto& app : dock_apps) {
            std::string app_content_dir = "/Applications/";
            app_content_dir += app;
            app_content_dir += ".app/Content/";
            auto json_parser = LFoundation::Json::Parser(app_content_dir + "info.json");
            LFoundation::Json::Object* jobj_root = json_parser.object();

            if (jobj_root->invalid()) {
                return;
            }

            auto* jdict_root = jobj_root->cast_to<LFoundation::Json::DictObject>();

            std::string icon_path = jdict_root->data()["icon_path"]->cast_to<LFoundation::Json::StringObject>()->data();
            std::string icon_rel_path = jdict_root->data()["icon_rel_path"]->cast_to<LFoundation::Json::StringObject>()->data();

            if (!icon_rel_path.empty()) {
                icon_path = app_content_dir + icon_rel_path;
            }
            std::string bundle_id = jdict_root->data()["bundle_id"]->cast_to<LFoundation::Json::StringObject>()->data();
            view().new_dock_entity(app_content_dir + app, icon_path, bundle_id);
        }
        
        view().set_needs_display();
    }

private:
    std::vector<std::string> dock_apps;
};