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

    if (is_hovered()) {
        auto rect = LG::Rect(padding, padding, DockView::icon_view_size() - 2 * padding, DockView::icon_view_size() - 2 * padding);
        ctx.set_fill_color(LG::Color::White);
        ctx.fill_rounded(rect, LG::CornerMask(6));
        ctx.set_fill_color(LG::Color(120, 129, 133, 60));
        ctx.draw_box_shading(rect, LG::Shading(LG::Shading::Type::Box, 0, 2), LG::CornerMask(6));
    }

    ctx.draw({ offset_x, offset_y }, m_icon);
}