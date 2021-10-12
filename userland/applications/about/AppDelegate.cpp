#include "ViewController.h"
#include <libui/AppDelegate.h>
#include <libui/MenuBar.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(200, 170); }
    const char* icon_path() const override { return "/res/icons/apps/about.icon"; }

    virtual bool application() override
    {
        auto& window = std::opuntiaos::construct<UI::Window>("About", window_size(), icon_path(), StatusBarStyle().set_hide_text());
        auto& superview = window.create_superview<UI::View, ViewController>();

        window.menubar().add_menu("Demo").add_item(UI::MenuItem("Say hello", [] { Logger::debug << "Hello!" << std::endl; }));

        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
