#include "ViewController.h"
#include <libui/AppDelegate.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(240, 340); }
    const char* icon_path() const override { return "/res/icons/apps/calculator.icon"; }

    virtual bool application() override
    {
        auto& window = std::opuntiaos::construct<UI::Window>("Calculator", window_size(), icon_path());
        auto& superview = window.create_superview<UI::View, ViewController>();
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);
