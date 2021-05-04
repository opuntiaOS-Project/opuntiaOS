#include "HomeScreenView.h"
#include "HomeScreenViewController.h"
#include "HomeScreenWindow.h"
#include <libui/AppDelegate.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(400, 300); }

    bool application() override
    {
        auto& window = std::oneos::construct<HomeScreenWindow>(window_size());
        window.set_bitmap_format(LG::PixelBitmapFormat::RGBA); // Turning on Alpha channel
        auto& dock_view = window.create_superview<HomeScreenView, HomeScreenViewController>();

        window.set_title("Homescreen");
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);