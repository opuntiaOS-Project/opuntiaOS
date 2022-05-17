#pragma once
#include "AppEntity.h"
#include "IconView.h"
#include <libg/Font.h>
#include <libui/CollectionView.h>
#include <libui/View.h>
#include <list>
#include <string>

class AppListView : public UI::View {
    UI_OBJECT();

public:
    AppListView(UI::View* superview, const LG::Rect& frame);
    AppListView(UI::View* superview, UI::Window* window, const LG::Rect& frame);

    static constexpr size_t padding() { return 16; }
    static constexpr int icon_size() { return 32; }
    static constexpr int icon_view_size() { return (int)64; }
    static constexpr int items_per_row() { return (int)4; }

    void display(const LG::Rect& rect) override;

    void register_entity(const AppEntity& ent);
    void register_entity(AppEntity&& ent);

private:
    UI::View* view_streamer(int id);

    UI::CollectionView* m_applist_grid_view {};
    std::vector<AppEntity> m_app_entities {};
};