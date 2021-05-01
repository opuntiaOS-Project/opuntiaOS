#include "DockView.h"
#include "DockViewController.h"
#include "DockWindow.h"
#include <libui/AppDelegate.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_window_size() const override { return LG::Size(400, 300); }

    bool application() override
    {
        auto& window = std::oneos::construct<DockWindow>();
        window.set_bitmap_format(LG::PixelBitmapFormat::RGBA); // Turning on Alpha channel
        auto& dock_view = window.create_superview<DockView, DockViewController>();

        window.set_title("Dock");
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);