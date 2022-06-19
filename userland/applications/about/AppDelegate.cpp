#include "ViewController.h"
#include <libui/AppDelegate.h>
#include <libui/MenuBar.h>
#include <libfoundation/AssetManager.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(200, 210); }
    const char* icon_path() const override {
        LFoundation::AssetManager assets = LFoundation::AssetManager("about");
        return assets.find("Resources/about.icon").c_str();
    }

    virtual bool application() override
    {
        auto style = StatusBarStyle(LG::Color(231, 240, 250)).set_hide_text();
        auto& window = std::opuntiaos::construct<UI::Window>("About", window_size(), icon_path(), style);
        auto& superview = window.create_superview<UI::View, ViewController>();

        auto demo_menu = UI::Menu("Demo");
        demo_menu.add_item(UI::MenuItem("Say hello", [] { Logger::debug << "Hello!" << std::endl; }));
        window.menubar().add_menu(std::move(demo_menu));

        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
