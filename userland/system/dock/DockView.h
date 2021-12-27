#pragma once
#include "AppListView.h"
#include "DockEntity.h"
#include "IconView.h"
#include "WindowEntity.h"
#include <libg/Font.h>
#include <libui/StackView.h>
#include <libui/View.h>
#include <list>
#include <string>

class DockView : public UI::View {
    UI_OBJECT();

public:
    DockView(UI::View* superview, const LG::Rect& frame);
    DockView(UI::View* superview, UI::Window* window, const LG::Rect& frame);

    static constexpr size_t padding() { return 0; }
    static constexpr size_t dock_view_height() { return 46; }
    static constexpr int icon_size() { return 32; }
    static constexpr int icon_view_size() { return (int)dock_view_height(); }

    void display(const LG::Rect& rect) override;

    WindowEntity* find_window_entry(int window_id);
    void on_window_create(const std::string& bundle_id, const std::string& icon_path, int window_id, int window_type);
    void on_window_remove(int window_id);
    void on_window_minimize(int window_id);
    void set_icon(int window_id, const std::string& path);
    void set_title(int window_id, const std::string& title);

    void add_system_buttons();

    void new_dock_entity(const std::string& exec_path, const std::string& icon_path, const std::string& bundle_id);

private:
    void launch(const DockEntity& ent);

    UI::StackView* m_dock_stackview {};
    AppListView* m_applist_view {};
    std::list<IconView*> m_icon_views {};
};