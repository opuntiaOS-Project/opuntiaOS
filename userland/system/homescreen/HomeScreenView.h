#pragma once
#include "AppListView.h"
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

    static constexpr int dock_height() { return 70; }
    static constexpr int applist_height() { return 22; }
    static constexpr int dock_height_with_padding() { return dock_height() + applist_height() + 2 * grid_padding() + 4; }
    static constexpr int grid_entities_size() { return 48; }
    static constexpr int icon_size() { return 48; }
    static constexpr int icon_view_size() { return 62; }
    static constexpr int grid_padding() { return 16; }
    static constexpr int grid_entities_per_row() { return 4; }
    static constexpr int grid_entities_per_column() { return 5; }

    void display(const LG::Rect& rect) override;

    void on_window_create(const std::string& bundle_id, const std::string& icon_path, int window_id, int window_type);

    void new_grid_entity(const std::string& title, const std::string& icon_path, std::string&& exec_path);
    void new_fast_launch_entity(const std::string& title, const std::string& icon_path, std::string&& exec_path);

private:
    std::vector<UI::StackView*> m_grid_stackviews;
    UI::StackView* m_dock_stackview {};
    AppListView* m_applist_view {};
    std::list<FastLaunchEntity> m_grid_entites {};
    std::list<FastLaunchEntity> m_fast_launch_entites {};
};