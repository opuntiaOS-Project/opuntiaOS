#include "AppListView.h"
#include "IconView.h"
#include <algorithm>
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/CollectionView.h>
#include <libui/Context.h>
#include <libui/StackView.h>
#include <libui/TextField.h>
#include <unistd.h>

static AppListView* this_view;

AppListView::AppListView(UI::View* superview, const LG::Rect& frame)
    : UI::View(superview, frame)
{
    // Not used any more, reinit
    // auto& dock_stack_view = add_subview<UI::StackView>(bounds());
    // dock_stack_view.set_spacing(padding());
    // dock_stack_view.set_background_color(LG::Color::Opaque);
    // dock_stack_view.set_axis(UI::LayoutConstraints::Axis::Horizontal);
    // dock_stack_view.set_distribution(UI::StackView::Distribution::EqualCentering);
    // m_dock_stackview = &dock_stack_view;
}

AppListView::AppListView(UI::View* superview, UI::Window* window, const LG::Rect& frame)
    : UI::View(superview, window, frame)
{
    // Use manual layouting since layout is really simple here.
    auto& header = add_subview<UI::View>(LG::Rect(0, 0, bounds().width(), 30 + 2 * padding()));
    header.set_background_color(LG::Color::White);
    header.layer().set_corner_mask(LG::CornerMask(8, LG::CornerMask::Masked, LG::CornerMask::NonMasked));

    auto& searchbar = add_subview<UI::TextField>(LG::Rect(padding(), padding(), bounds().width() - padding() * 2, 30));
    searchbar.layer().set_corner_mask(LG::CornerMask(8));
    searchbar.set_background_color(LG::Color(0xE7F0FA));
    searchbar.set_placeholder_text("SEARCH");

    auto& applist_grid_view = add_subview<UI::CollectionView>(LG::Rect(0, 2 * padding() + 30, bounds().width(), bounds().height() - (2 * padding() + 30)));
    applist_grid_view.set_data_source([this](int id) -> View* {
        return this->view_streamer(id);
    });
    m_applist_grid_view = &applist_grid_view;
}

void AppListView::display(const LG::Rect& rect)
{
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill_rounded(bounds(), layer().corner_mask());

    ctx.set_fill_color(LG::Color(120, 120, 120, 50));
    ctx.draw_shading(LG::Rect(0, 30 + 2 * padding(), bounds().width(), 4), LG::Shading(LG::Shading::Type::TopToBottom));
}

UI::View* AppListView::view_streamer(int id)
{
    if (id * items_per_row() >= m_app_entities.size()) {
        return nullptr;
    }

    size_t calc_padding = (m_applist_grid_view->bounds().width() - 2 * padding() - (icon_view_size() * items_per_row())) / (items_per_row() - 1);
    UI::StackView& dock_stack_view = m_applist_grid_view->add_subview<UI::StackView>(LG::Rect(padding(), 0, m_applist_grid_view->bounds().width() - 2 * padding(), icon_view_size() + calc_padding));
    dock_stack_view.set_spacing(calc_padding);
    dock_stack_view.set_background_color(LG::Color::Opaque);
    dock_stack_view.set_axis(UI::LayoutConstraints::Axis::Horizontal);
    dock_stack_view.set_distribution(UI::StackView::Distribution::Standard);

    size_t rem = std::min(items_per_row(), (int)m_app_entities.size() - id * items_per_row());
    for (int i = 0; i < rem; i++) {
        auto& icon_view = dock_stack_view.add_arranged_subview<IconView>();
        icon_view.set_title(m_app_entities[id * items_per_row() + i].title());
        icon_view.entity() = m_app_entities[id * items_per_row() + i];
        icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, icon_view_size()));
        icon_view.add_constraint(UI::Constraint(icon_view, UI::Constraint::Attribute::Width, UI::Constraint::Relation::Equal, icon_view_size()));
    }

    return &dock_stack_view;
}

void AppListView::register_entity(const AppEntity& ent)
{
    m_app_entities.push_back(ent);
    m_applist_grid_view->reload_data();
    m_applist_grid_view->invalidate_row((m_app_entities.size() - 1) / items_per_row());
}

void AppListView::register_entity(AppEntity&& ent)
{
    m_app_entities.push_back(std::move(ent));
    m_applist_grid_view->reload_data();
    m_applist_grid_view->invalidate_row((m_app_entities.size() - 1) / items_per_row());
}
