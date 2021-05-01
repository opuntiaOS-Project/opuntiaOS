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
static int icons_per_line = 4;
static int icons_per_column = 5;

HomeScreenView::HomeScreenView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

void HomeScreenView::display(const LG::Rect& rect)
{
    // FIXME: Rendering is a bit studip now, will rewrite it
    //        completely when stackview is available.
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    ctx.set_fill_color(LG::Color(222, 222, 222, 0));
    ctx.fill(bounds());

    // Drawing launched icons
    int spacing_x = (bounds().width() - 48 * icons_per_line) / 6;
    int spacing_y = (bounds().height() - 48 * icons_per_column) / 7;
    int offsetx = spacing_x + 4;
    int offsety = spacing_y;
    int drawn = 0;
    for (auto& entity : m_fast_launch_entites) {
        ctx.draw({ offsetx, offsety }, entity.icon());
        offsetx += entity.icon().bounds().width() + spacing_x;
        drawn++;
        if (drawn == icons_per_line) {
            offsetx = spacing_x + 4;
            offsety += spacing_y;
        }
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
    int spacing_x = (bounds().width() - 48 * icons_per_line) / 6;
    int spacing_y = (bounds().height() - 48 * icons_per_column) / 7;
    int offsetx = spacing_x + 4;
    int offsety = spacing_y;
    int drawn = 0;
    for (auto& entity : m_fast_launch_entites) {
        auto it = LG::Rect(offsetx, offsety, 48, 48);
        if (it.contains(location)) {
            launch(entity);
        }
        offsetx += entity.icon().bounds().width() + spacing_x;
        drawn++;
        if (drawn == icons_per_line) {
            offsetx = spacing_x + 4;
            offsety += spacing_y;
        }
    }
}