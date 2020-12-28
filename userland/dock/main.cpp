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
    auto& dock_view = window.create_superview<DockView>();
    dock_view.set_background_color(0x08090C);
    window.set_title("Dock");
    return app.run();
}