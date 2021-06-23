#include "IconView.h"
#include "HomeScreenView.h"
#include <libui/Context.h>
#include <libui/Label.h>

IconView::IconView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
    m_label = &add_subview<UI::Label>(LG::Rect(0, HomeScreenView::icon_view_size() - 12, HomeScreenView::icon_view_size(), 12));
    m_label->set_alignment(UI::Text::Alignment::Center);
}

void IconView::display(const LG::Rect& rect)
{
    const int offset_x = (HomeScreenView::icon_view_size() - HomeScreenView::icon_size()) / 2;
    UI::Context ctx(*this);
    ctx.add_clip(rect);
    ctx.draw({ offset_x, 0 }, m_launch_entity.icon());
}