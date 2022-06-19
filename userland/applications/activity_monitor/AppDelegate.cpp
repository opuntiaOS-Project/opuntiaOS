#include "ViewController.h"
#include <libui/AppDelegate.h>
#include <libfoundation/AssetManager.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(220, 210); }
    const char* icon_path() const override {
        LFoundation::AssetManager assets = LFoundation::AssetManager("activity_monitor");
        return assets.find("Resources/activity_monitor.icon").c_str();
    }

    virtual bool application() override
    {
        auto style = StatusBarStyle(LG::Color(222, 232, 227)).set_hide_text();
        auto& window = std::opuntiaos::construct<UI::Window>("Monitor", window_size(), icon_path(), style);
        auto& superview = window.create_superview<UI::View, ViewController>();
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
