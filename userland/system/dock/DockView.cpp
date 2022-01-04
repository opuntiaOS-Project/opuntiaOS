#include "DockView.h"
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

static DockView* this_view;

DockView::DockView(UI::View* superview, const LG::Rect& frame)
    : UI::View(superview, frame)
{
    init_fill_bounds();
    init_subviews();
}

DockView::DockView(UI::View* superview, UI::Window* window, const LG::Rect& frame)
    : UI::View(superview, window, frame)
{
    init_fill_bounds();
    init_subviews();
}

void DockView::init_fill_bounds()
{
    m_fill_bounds = bounds();
    m_fill_bounds.set_x(bounds().mid_x());
    m_fill_bounds.set_width(0);
    fill_bounds_expand(2 * padding());
}

void DockView::init_subviews()
{
    auto& dock_stack_view = add_subview<UI::StackView>(bounds());
    dock_stack_view.set_spacing(padding());
    dock_stack_view.set_background_color(LG::Color::Opaque);
    dock_stack_view.set_axis(UI::LayoutConstraints::Axis::Horizontal);
    dock_stack_view.set_distribution(UI::StackView::Distribution::EqualCentering);
    m_dock_stackview = &dock_stack_view;
    add_system_buttons();
}

void DockView::add_system_buttons()
{
    fill_bounds_expand(icon_view_size() + padding());
    auto& icon_view = m_dock_stackview->add_arranged_subview<AppListView>();
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, icon_view_size()));
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Width, UI::Constraint::Relation::Equal, icon_view_size()));
    m_applist_view = &icon_view;
}

void DockView::display(const LG::Rect& rect)
{
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill_rounded(fill_bounds(), LG::CornerMask(4, LG::CornerMask::Masked, LG::CornerMask::NonMasked));
}

void DockView::new_dock_entity(const std::string& exec_path, const std::string& icon_path, const std::string& bundle_id)
{
    LG::PNG::PNGLoader loader;

    fill_bounds_expand(icon_view_size() + padding());
    auto& icon_view = m_dock_stackview->add_arranged_subview<IconView>();
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, icon_view_size()));
    icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Width, UI::Constraint::Relation::Equal, icon_view_size()));
    icon_view.entity().set_icon(loader.load_from_file(icon_path + "/32x32.png"));
    icon_view.entity().set_path_to_exec(std::move(exec_path));
    icon_view.entity().set_bundle_id(std::move(bundle_id));
    m_icon_views.push_back(&icon_view);
    set_needs_layout();
}

WindowEntity* DockView::find_window_entry(int window_id)
{
    for (auto* view : m_icon_views) {
        for (auto& wins : view->entity().windows()) {
            if (wins.window_id() == window_id) {
                return &wins;
            }
        }
    }
    return nullptr;
}

void DockView::on_window_create(const std::string& bundle_id, const std::string& icon_path, int window_id, int window_type)
{
    // Don't add an icon of dock (self).
    if (window()->id() == window_id) {
        return;
    }

    if (window_type == WindowType::AppList) {
        if (m_applist_view) {
            m_applist_view->set_target_window_id(window_id);
        }
        return;
    }

    for (auto* view : m_icon_views) {
        if (view->entity().bundle_id() == bundle_id) {
            view->entity().add_window(WindowEntity(window_id));
            set_needs_display();
            return;
        }
    }

    new_dock_entity("", icon_path, bundle_id);
    for (auto* view : m_icon_views) {
        if (view->entity().bundle_id() == bundle_id) {
            view->entity().add_window(WindowEntity(window_id));
            set_needs_display();
            return;
        }
    }
}

void DockView::on_window_minimize(int window_id)
{
    auto* ent = find_window_entry(window_id);
    if (!ent) {
        return;
    }
    ent->set_minimized(true);
    set_needs_display();
}

void DockView::on_window_remove(int window_id)
{
    // TODO: Currently icons are not properly deleted.
    for (auto* view : m_icon_views) {
        for (auto& wins : view->entity().windows()) {
            if (wins.window_id() == window_id) {
                auto& win = view->entity().windows();
                win.erase(std::find(win.begin(), win.end(), WindowEntity(window_id)));
                set_needs_display();
                return;
            }
        }
    }
}

void DockView::set_icon(int window_id, const std::string& path)
{
}

void DockView::set_title(int window_id, const std::string& title)
{
    auto* ent = find_window_entry(window_id);
    if (!ent) {
        return;
    }
    ent->set_title(title);
}
