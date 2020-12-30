#pragma once
#include "DockEntity.h"
#include "FastLaunchEntity.h"
#include <libg/Font.h>
#include <libui/View.h>
#include <std/LinkedList.h>
#include <std/String.h>

class DockView : public UI::View {
public:
    DockView(const LG::Rect&);

    void display(const LG::Rect& rect) override;
    void click_began(const LG::Point<int>& location) override;

    DockEntity* find_entity(int window_id);
    void new_entity(int window_id);
    void remove_entity(int window_id);
    void set_icon(int window_id, const LG::String& path);

    void new_fast_launch_entity(const LG::String& icon_path, LG::String&& exec_path);

private:
    void launch(const FastLaunchEntity& ent);

    int m_fast_launch_entites_count { 0 };
    LinkedList<FastLaunchEntity> m_fast_launch_entites {};
    LinkedList<DockEntity> m_dock_entites {};
};