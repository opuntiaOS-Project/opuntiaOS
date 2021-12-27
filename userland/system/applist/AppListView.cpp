#include "AppListView.h"
#include "IconView.h"
#include <algorithm>
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Context.h>
#include <unistd.h>

static AppListView* this_view;

AppListView::AppListView(UI::View* superview, const LG::Rect& frame)
    : UI::View(superview, frame)
{
    auto& dock_stack_view = add_subview<UI::StackView>(bounds());
    dock_stack_view.set_spacing(padding());
    dock_stack_view.set_background_color(LG::Color::Opaque);
    dock_stack_view.set_axis(UI::LayoutConstraints::Axis::Horizontal);
    dock_stack_view.set_distribution(UI::StackView::Distribution::EqualCentering);
    m_dock_stackview = &dock_stack_view;
}

AppListView::AppListView(UI::View* superview, UI::Window* window, const LG::Rect& frame)
    : UI::View(superview, window, frame)
{
    auto& label = add_subview<UI::Label>(LG::Rect(16, 16, 16, 24));
    label.set_text_color(LG::Color::DarkSystemText);
    label.set_text("App List");
    label.set_font(LG::Font::system_font(LG::Font::SystemTitleSize));
    label.set_width(label.preferred_width());

    auto& dock_stack_view = add_subview<UI::StackView>(LG::Rect(0, 50, bounds().width(), bounds().height()));
    dock_stack_view.set_spacing(padding());
    dock_stack_view.set_background_color(LG::Color::Opaque);
    dock_stack_view.set_axis(UI::LayoutConstraints::Axis::Horizontal);
    dock_stack_view.set_distribution(UI::StackView::Distribution::EqualCentering);
    m_dock_stackview = &dock_stack_view;
}

void AppListView::display(const LG::Rect& rect)
{
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill_rounded(bounds(), layer().corner_mask());
}

void AppListView::new_dock_entity(const std::string& exec_path, const std::string& icon_path, const std::string& bundle_id)
{
    LG::PNG::PNGLoader loader;

    auto& icon_view = m_dock_stackview->add_arranged_subview<IconView>();
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, icon_view_size()));
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Width, UI::Constraint::Relation::Equal, icon_view_size()));
    icon_view.entity().set_icon(loader.load_from_file(icon_path + "/48x48.png"));
    icon_view.entity().set_path_to_exec(std::move(exec_path));
    icon_view.entity().set_bundle_id(std::move(bundle_id));
    m_icon_views.push_back(&icon_view);
    set_needs_layout();
}
