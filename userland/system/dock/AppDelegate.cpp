#include "DockView.h"
#include "DockViewController.h"
#include "DockWindow.h"
#include <libui/AppDelegate.h>

class AppDelegate : public UI::AppDelegate {
public:
    AppDelegate() = default;
    virtual ~AppDelegate() = default;

    LG::Size preferred_desktop_window_size() const override { return LG::Size(400, 300); }

    bool application() override
    {
        auto& window = std::opuntiaos::construct<DockWindow>();
        window.set_bitmap_format(LG::PixelBitmapFormat::RGBA);
        auto& dock_view = window.create_superview<DockView, DockViewController>();
        return true;
    }

private:
};

SET_APP_DELEGATE(AppDelegate);