#pragma once
#include "DockEntity.h"
#include "IconView.h"
#include "WindowEntity.h"
#include <libg/Font.h>
#include <libui/StackView.h>
#include <libui/View.h>
#include <list>
#include <string>

class AppListView : public UI::View {
    UI_OBJECT();

public:
    AppListView(UI::View* superview, const LG::Rect& frame);
    AppListView(UI::View* superview, UI::Window* window, const LG::Rect& frame);

    static constexpr size_t padding() { return 0; }
    static constexpr size_t dock_view_height() { return 46; }
    static constexpr int icon_size() { return 48; }
    static constexpr int icon_view_size() { return (int)60; }

    void display(const LG::Rect& rect) override;

    void new_dock_entity(const std::string& exec_path, const std::string& icon_path, const std::string& bundle_id);

private:
    void launch(const DockEntity& ent);

    UI::StackView* m_dock_stackview {};
    std::list<IconView*> m_icon_views {};
};