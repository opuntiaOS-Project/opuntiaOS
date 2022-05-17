#pragma once
#include <libg/Font.h>
#include <libui/Label.h>
#include <libui/PopupMenu.h>
#include <libui/View.h>
#include <list>
#include <string>
#include <unistd.h>

class AppListView : public UI::View {
    UI_OBJECT();

    static constexpr int INVALID = -1;

public:
    AppListView(View* superview, const LG::Rect& frame);

    void set_target_window_id(int winid) { m_target_window_id = winid; }

    void display(const LG::Rect& rect) override;
    void mouse_up() override { on_click(); }

    void on_click();

private:
    int m_target_window_id { INVALID };
    LG::PixelBitmap m_icon;
};