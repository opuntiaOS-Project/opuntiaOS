#include "HomeScreenView.h"
#include <algorithm>
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Context.h>
#include <unistd.h>

static HomeScreenView* this_view;

HomeScreenView::HomeScreenView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

void HomeScreenView::display(const LG::Rect& rect)
{
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    ctx.set_fill_color(LG::Color(222, 222, 222, 0));
    ctx.fill(bounds());

    // Drawing launched icons
    int spacing_x = (bounds().width() - grid_entities_size() * grid_entities_per_row()) / (grid_entities_per_row() + 2);
    int spacing_y = (bounds().height() - dock_height() - grid_entities_size() * grid_entities_per_column()) / (grid_entities_per_column() + 2);
    int offsetx = spacing_x + 4;
    int offsety = spacing_y;
    int drawn = 0;
    for (auto& entity : m_fast_launch_entites) {
        ctx.draw({ offsetx, offsety }, entity.icon());
        offsetx += spacing_x + grid_entities_size();
        drawn++;
        if (drawn == grid_entities_per_row()) {
            offsetx = spacing_x + 4;
            offsety += grid_entities_size() + spacing_y;
        }
    }

    ctx.set_fill_color(LG::Color(222, 222, 222, 180));
    offsety = bounds().height() - dock_height();
    ctx.fill(LG::Rect(0, offsety, bounds().width(), dock_height()));
    offsetx = (bounds().width() - (grid_entities_size() * m_fast_launch_entites.size() + spacing_x * (m_fast_launch_entites.size() - 1))) / 2;
    offsety += 10;
    for (auto& entity : m_fast_launch_entites) {
        ctx.draw({ offsetx, offsety }, entity.icon());
        offsetx += spacing_x + grid_entities_size();
    }
}

void HomeScreenView::new_fast_launch_entity(const LG::string& icon_path, LG::string&& exec_path)
{
    LG::PNG::PNGLoader loader;
    m_fast_launch_entites.push_back(FastLaunchEntity());
    m_fast_launch_entites.back().set_icon(loader.load_from_file(icon_path + "/48x48.png"));
    m_fast_launch_entites.back().set_path_to_exec(std::move(exec_path));
    set_needs_display();
}

void HomeScreenView::launch(const FastLaunchEntity& ent)
{
    if (fork() == 0) {
        execve(ent.path_to_exec().c_str(), 0, 0);
        std::abort();
    }
}

void HomeScreenView::click_began(const LG::Point<int>& location)
{
    int spacing_x = (bounds().width() - grid_entities_size() * grid_entities_per_row()) / (grid_entities_per_row() + 2);
    int spacing_y = (bounds().height() - dock_height() - grid_entities_size() * grid_entities_per_column()) / (grid_entities_per_column() + 2);
    int offsetx = spacing_x + 4;
    int offsety = spacing_y;
    int drawn = 0;
    for (auto& entity : m_fast_launch_entites) {
        auto it = LG::Rect(offsetx, offsety, grid_entities_size(), grid_entities_size());
        if (it.contains(location)) {
            launch(entity);
        }
        offsetx += spacing_x + grid_entities_size();
        drawn++;
        if (drawn == grid_entities_per_row()) {
            offsetx = spacing_x + 4;
            offsety += spacing_y + grid_entities_size();
        }
    }

    offsety = bounds().height() - dock_height();
    offsetx = (bounds().width() - (grid_entities_size() * m_fast_launch_entites.size() + spacing_x * (m_fast_launch_entites.size() - 1))) / 2;
    offsety += 10;
    for (auto& entity : m_fast_launch_entites) {
        auto it = LG::Rect(offsetx, offsety, grid_entities_size(), grid_entities_size());
        if (it.contains(location)) {
            launch(entity);
        }
        offsetx += spacing_x + grid_entities_size();
    }
}