#include "AppListView.h"
#include "DockView.h"
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Context.h>
#include <libui/Label.h>
#include <libui/Screen.h>
#include <libui/Window.h>

AppListView::AppListView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
    LG::PNG::PNGLoader loader;
    m_icon = loader.load_from_file("/res/system/app_list_32.png");
}

void AppListView::display(const LG::Rect& rect)
{
    const int padding = 4;
    const int offset_x = (DockView::icon_view_size() - m_icon.width()) / 2;
    const int offset_y = (DockView::icon_view_size() - m_icon.height()) / 2;
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    auto icon_rect = LG::Rect(offset_x, offset_y, DockView::icon_size(), DockView::icon_size());
    ctx.draw(icon_rect.origin(), m_icon);
    if (is_hovered()) {
        ctx.set_fill_color(LG::Color::LightSystemOpaque128);
        ctx.fill(icon_rect);
    }
}

void AppListView::on_click()
{
    int this_window_id = window()->id();

    if (m_target_window_id == INVALID) {
        return;
    }

    auto& app = UI::App::the();
    AskBringToFrontMessage msg(app.connection().key(), this_window_id, m_target_window_id);
    app.connection().send_async_message(msg);
}