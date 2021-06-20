#pragma once
#include "DockEntity.h"
#include "FastLaunchEntity.h"
#include <libg/Font.h>
#include <libui/View.h>
#include <list>
#include <string>

class DockView : public UI::View {
    UI_OBJECT();

public:
    DockView(UI::View* superview, const LG::Rect& frame);
    DockView(UI::View* superview, UI::Window* window, const LG::Rect& frame);

    void display(const LG::Rect& rect) override;
    void click_began(const LG::Point<int>& location) override;

    DockEntity* find_entity(int window_id);
    void new_entity(int window_id);
    void remove_entity(int window_id);
    void set_icon(int window_id, const LG::string& path);

    void new_fast_launch_entity(const LG::string& icon_path, LG::string&& exec_path);

private:
    void launch(const FastLaunchEntity& ent);

    std::list<FastLaunchEntity> m_fast_launch_entites {};
    std::list<DockEntity> m_dock_entites {};
};