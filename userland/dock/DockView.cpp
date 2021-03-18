#include "DockView.h"
#include <algorithm>
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Context.h>
#include <std/Dbg.h>
#include <unistd.h>

static DockView* this_view;

DockView::DockView(const LG::Rect& frame)
    : View(frame)
{
}

void DockView::display(const LG::Rect& rect)
{
    // FIXME: Rendering is a bit studip now, will rewrite it
    //        completely when stackview is available.
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    // Drawing fast-launch icons
    ctx.set_fill_color(background_color());
    ctx.fill(bounds());

    ctx.set_fill_color(LG::Color(222, 222, 222, 255));
    ctx.fill(LG::Rect(0, 0, 8 + 40 * m_fast_launch_entites.size(), bounds().height()));

    // Drawing launched icons
    int offsetx = 8;
    for (auto& entity : m_fast_launch_entites) {
        ctx.draw({ offsetx, 2 }, entity.icon());
        offsetx += entity.icon().bounds().width() + 8;
    }

    offsetx += 8;
    for (auto& entity : m_dock_entites) {
        ctx.draw({ offsetx, 2 }, entity.icon());
        ctx.fill(LG::Rect(offsetx, 34, 32, 2));
        offsetx += entity.icon().bounds().width() + 8;
    }
}

void DockView::new_fast_launch_entity(const LG::string& icon_path, LG::string&& exec_path)
{
    LG::PNG::PNGLoader loader;
    m_fast_launch_entites.push_back(FastLaunchEntity());
    m_fast_launch_entites.back().set_icon(loader.load_from_file(icon_path + "/32x32.png"));
    m_fast_launch_entites.back().set_path_to_exec(std::move(exec_path));
    set_needs_display();
}

DockEntity* DockView::find_entity(int window_id)
{
    for (auto& ent : m_dock_entites) {
        if (ent.window_id() == window_id) {
            return &ent;
        }
    }
    return nullptr;
}

void DockView::new_entity(int window_id)
{
    // Don't add an icon of dock (self).
    if (window()->id() != window_id) {
        m_dock_entites.push_back(DockEntity(window_id));
    }
}

void DockView::remove_entity(int window_id)
{
    m_dock_entites.erase(std::find(m_dock_entites.begin(), m_dock_entites.end(), DockEntity(window_id)));
    set_needs_display();
}

void DockView::set_icon(int window_id, const LG::string& path)
{
    auto* ent = find_entity(window_id);
    if (!ent) {
        return;
    }
    LG::PNG::PNGLoader loader;
    ent->set_icon(loader.load_from_file(path + "/32x32.png"));
    set_needs_display();
}

void DockView::launch(const FastLaunchEntity& ent)
{
    if (fork() == 0) {
        execve(ent.path_to_exec().c_str(), 0, 0);
        std::abort();
    }
}

void DockView::click_began(const LG::Point<int>& location)
{
    // Check if it is a tap on fast-launch icons
    int offsetx = 8;
    for (auto& entity : m_fast_launch_entites) {
        auto it = LG::Rect(offsetx, 2, 32, 32);
        if (it.contains(location)) {
            launch(entity);
        }
        offsetx += entity.icon().bounds().width() + 8;
    }

    offsetx += 8;
    for (auto& entity : m_dock_entites) {
        auto it = LG::Rect(offsetx, 2, 32, 32);
        if (it.contains(location)) {
            auto& app = UI::App::the();
            AskBringToFrontMessage msg(app.connection().key(), window()->id(), entity.window_id());
            app.connection().send_async_message(msg);
            return;
        }
        offsetx += entity.icon().bounds().width() + 8;
    }
}