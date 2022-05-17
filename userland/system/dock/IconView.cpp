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

    auto icon_rect = LG::Rect(offset_x, offset_y, DockView::icon_size(), DockView::icon_size());
    ctx.draw(icon_rect.origin(), m_launch_entity.icon());
    if (is_hovered()) {
        ctx.set_fill_color(LG::Color::LightSystemOpaque128);
        ctx.fill(icon_rect);
    }

    ctx.set_fill_color(LG::Color(120, 129, 133, 40));
    ctx.draw_box_shading(icon_rect, LG::Shading(LG::Shading::Type::Box, 0, 3), LG::CornerMask(6));

    ctx.set_fill_color(LG::Color(163, 174, 190));
    const int underline_y = DockView::icon_view_size() - underline_height() - padding;
    if (entity().windows().size() > 0) {
        const int len = 8;
        ctx.fill({ (DockView::icon_view_size() - len) / 2, underline_y, len, underline_height() });
    }
}

void IconView::on_click()
{
    if (entity().windows().empty()) {
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
                for (auto& win : entity().windows()) {
                    if (win.window_id() == target_window_id) {
                        win.set_minimized(false);
                        break;
                    }
                }

                auto& app = UI::App::the();
                AskBringToFrontMessage msg(app.connection().key(), this_window_id, target_window_id);
                app.connection().send_async_message(msg);
            }));
        }
        demo_menu.add_item(UI::MenuItem("New window", [this] {
            launch();
        }));
        window()->popup_manager().show({ frame().min_x(), (int)UI::Screen::main().bounds().height() - (int)DockView::dock_view_height() - 4 }, demo_menu);
    }
}