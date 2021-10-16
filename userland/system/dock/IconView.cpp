#include "IconView.h"
#include "DockView.h"
#include <libui/App.h>
#include <libui/Context.h>
#include <libui/Label.h>
#include <libui/Screen.h>
#include <libui/Window.h>

IconView::IconView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

void IconView::display(const LG::Rect& rect)
{
    const int padding = 4;
    const int offset_x = (DockView::icon_view_size() - DockView::icon_size()) / 2;
    const int offset_y = (DockView::icon_view_size() - DockView::icon_size()) / 2;
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    if (is_hovered()) {
        auto rect = LG::Rect(padding, padding, DockView::icon_view_size() - 2 * padding, DockView::icon_view_size() - 2 * padding);
        ctx.set_fill_color(LG::Color::White);
        ctx.fill_rounded(rect, LG::CornerMask(6));
        ctx.set_fill_color(LG::Color(120, 129, 133, 80));
        ctx.draw_box_shading(rect, LG::Shading(LG::Shading::Type::Box, 0, 4), LG::CornerMask(6));
    }

    ctx.draw({ offset_x, offset_y }, m_launch_entity.icon());

    ctx.set_fill_color(LG::Color(120, 129, 133));
    const int underline_y = DockView::icon_view_size() - underline_height() - padding;
    if (entity().windows().size() > 1) {
        const int len = 10;
        ctx.fill({ (DockView::icon_view_size() - len) / 2, underline_y, len - underline_height(), underline_height() });
        ctx.fill({ (DockView::icon_view_size() - len) / 2 + len, underline_y, underline_height(), underline_height() });
    } else if (entity().windows().size() > 0) {
        const int len = 10;
        ctx.fill({ (DockView::icon_view_size() - len) / 2, underline_y, len, underline_height() });
    }
}

void IconView::on_click()
{
    if (entity().windows().size() == 0) {
        launch();
        return;
    } else {
        auto demo_menu = UI::Menu();
        for (auto& win : entity().windows()) {
            auto title = win.title();
            int this_window_id = window()->id();
            int target_window_id = win.window_id();

            if (win.is_minimized()) {
                title += " - minimized";
            }

            demo_menu.add_item(UI::MenuItem(title, [this, this_window_id, target_window_id] {
                auto& app = UI::App::the();
                AskBringToFrontMessage msg(app.connection().key(), this_window_id, target_window_id);
                // this->entity().set_minimized(false);
                app.connection().send_async_message(msg);
            }));
        }
        demo_menu.add_item(UI::MenuItem("New window", [this] {
            launch();
        }));
        window()->popup_manager().show({ frame().min_x(), (int)UI::Screen::main().bounds().height() - (int)DockView::dock_view_height() - 4 }, demo_menu);
    }
}