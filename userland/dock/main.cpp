#include "DockView.h"
#include "DockWindow.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <std/Dbg.h>

int main(int argc, char** argv)
{
    auto& app = create<UI::App>();
    auto& window = create<DockWindow>();
    window.set_bitmap_format(LG::PixelBitmapFormat::RGBA); // Turning on Alpha channel
    auto& dock_view = window.create_superview<DockView>();
    dock_view.set_background_color(LG::Color(222, 222, 222, 180));
    dock_view.new_fast_launch_entity("/res/icons/apps/about.icon", "/bin/about"); // FIXME: Parse some init file
    window.set_title("Dock");
    return app.run();
}