#pragma once
#include "FastLaunchEntity.h"
#include "HomeScreenEntity.h"
#include <libg/Font.h>
#include <libui/View.h>
#include <list>
#include <string>

class HomeScreenView : public UI::View {
    UI_OBJECT();

public:
    HomeScreenView(View* superview, const LG::Rect& frame);

    static constexpr int dock_height() { return 80; }
    static constexpr int grid_entities_size() { return 48; }
    static constexpr int grid_entities_per_row() { return 4; }
    static constexpr int grid_entities_per_column() { return 5; }

    void display(const LG::Rect& rect) override;
    void click_began(const LG::Point<int>& location) override;

    void new_fast_launch_entity(const LG::string& icon_path, LG::string&& exec_path);

private:
    void launch(const FastLaunchEntity& ent);

    std::list<FastLaunchEntity> m_grid_entites {};
    std::list<FastLaunchEntity> m_fast_launch_entites {};
};