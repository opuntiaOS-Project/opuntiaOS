/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "AppListView.h"
#include <libfoundation/FileManager.h>
#include <libfoundation/json/Parser.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

char contentdir[256];

class AppListViewController : public UI::ViewController<AppListView> {
public:
    AppListViewController(AppListView& view)
        : UI::ViewController<AppListView>(view)
    {
    }
    virtual ~AppListViewController() = default;

    void load_application(const std::string& content_dir)
    {
        auto json_parser = LFoundation::Json::Parser(content_dir + "info.json");
        LFoundation::Json::Object* jobj_root = json_parser.object();
        if (jobj_root->invalid()) {
            return;
        }

        auto* jdict_root = jobj_root->cast_to<LFoundation::Json::DictObject>();
        const std::string& bundle_id = jdict_root->data()["bundle_id"]->cast_to<LFoundation::Json::StringObject>()->data();

        AppEntity new_ent;
        LG::PNG::PNGLoader loader;

        std::string icon_path = jdict_root->data()["icon_path"]->cast_to<LFoundation::Json::StringObject>()->data();
        std::string icon_rel_path = jdict_root->data()["icon_rel_path"]->cast_to<LFoundation::Json::StringObject>()->data();

        if (!icon_rel_path.empty()) {
            icon_path = content_dir + icon_rel_path;
        }

        new_ent.set_icon(loader.load_from_file(icon_path + "/32x32.png"));

        std::string rel_exec_path = jdict_root->data()["exec_rel_path"]->cast_to<LFoundation::Json::StringObject>()->data();
        new_ent.set_path_to_exec(content_dir + rel_exec_path);

        new_ent.set_title(jdict_root->data()["name"]->cast_to<LFoundation::Json::StringObject>()->data());
        new_ent.set_bundle_id(jdict_root->data()["bundle_id"]->cast_to<LFoundation::Json::StringObject>()->data());
        view().register_entity(std::move(new_ent));

        delete jdict_root;
    }

    void load_application_list()
    {
        auto local_fm = LFoundation::FileManager();
        local_fm.foreach_object("/Applications", [this](const char* name) {
            sprintf(contentdir, "/Applications/%s/Content/", name);
            load_application(contentdir);
        });
    }

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemOpaque);
        view().layer().set_corner_mask(LG::CornerMask(4, LG::CornerMask::Masked, LG::CornerMask::NonMasked));
        load_application_list();
#if 0
        for (int i = 0; i < 32; i++) {
            AppEntity new_ent;
            LG::PNG::PNGLoader loader;

            new_ent.set_icon(loader.load_from_file("/res/icons/apps/about.icon/32x32.png"));
            new_ent.set_path_to_exec("/Applications/about.app/Content/about");
            new_ent.set_title("TestApp");
            new_ent.set_bundle_id("com.opuntia.test");
            view().register_entity(std::move(new_ent));
        }
#endif
        view().set_needs_display();
    }

private:
};