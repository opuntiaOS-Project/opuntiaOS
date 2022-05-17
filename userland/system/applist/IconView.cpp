#include "IconView.h"
#include "AppListView.h"
#include <libui/App.h>
#include <libui/Context.h>
#include <libui/Label.h>
#include <libui/Screen.h>
#include <libui/Window.h>

IconView::IconView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
    m_label = &add_subview<UI::Label>(LG::Rect(0, AppListView::icon_view_size() - 12, AppListView::icon_view_size(), 12));
    m_label->set_alignment(UI::Text::Alignment::Center);
}

void IconView::display(const LG::Rect& rect)
{
    const int padding = 4;
    const int offset_x = (AppListView::icon_view_size() - AppListView::icon_size()) / 2;
    const int offset_y = (AppListView::icon_view_size() - AppListView::icon_size()) / 2;
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    auto icon_rect = LG::Rect(offset_x, offset_y, AppListView::icon_size(), AppListView::icon_size());
    ctx.draw(icon_rect.origin(), m_launch_entity.icon());
    if (is_hovered()) {
        ctx.set_fill_color(LG::Color::LightSystemOpaque128);
        ctx.fill(icon_rect);
    }

    ctx.set_fill_color(LG::Color(120, 129, 133, 40));
    ctx.draw_box_shading(icon_rect, LG::Shading(LG::Shading::Type::Box, 0, 3), LG::CornerMask(6));
}

void IconView::on_click()
{
    launch();
}