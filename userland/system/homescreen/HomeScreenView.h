#pragma once
#include "FastLaunchEntity.h"
#include "HomeScreenEntity.h"
#include <libg/Font.h>
#include <libui/StackView.h>
#include <libui/View.h>
#include <list>
#include <string>
#include <vector>

class HomeScreenView : public UI::View {
    UI_OBJECT();

public:
    HomeScreenView(UI::View* superview, const LG::Rect& frame);
    HomeScreenView(UI::View* superview, UI::Window* window, const LG::Rect& frame);

    static constexpr int dock_height() { return 80; }
    static constexpr int dock_height_with_padding() { return dock_height() + grid_padding() + 4; }
    static constexpr int grid_entities_size() { return 48; }
    static constexpr int icon_size() { return 48; }
    static constexpr int icon_view_size() { return 62; }
    static constexpr int grid_padding() { return 16; }
    static constexpr int grid_entities_per_row() { return 4; }
    static constexpr int grid_entities_per_column() { return 5; }

    void display(const LG::Rect& rect) override;
    
    void new_grid_entity(const LG::string& title, const LG::string& icon_path, LG::string&& exec_path);
    void new_fast_launch_entity(const LG::string& title, const LG::string& icon_path, LG::string&& exec_path);

private:

    std::vector<UI::StackView*> m_grid_stackviews;
    UI::StackView* m_dock_stackview {};
    std::list<FastLaunchEntity> m_grid_entites {};
    std::list<FastLaunchEntity> m_fast_launch_entites {};
};