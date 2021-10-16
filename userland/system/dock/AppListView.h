#pragma once
#include "DockEntity.h"
#include "WindowEntity.h"
#include <libg/Font.h>
#include <libui/Label.h>
#include <libui/PopupMenu.h>
#include <libui/View.h>
#include <list>
#include <string>
#include <unistd.h>

class AppListView : public UI::View {
    UI_OBJECT();

public:
    AppListView(View* superview, const LG::Rect& frame);

    void display(const LG::Rect& rect) override;
    void mouse_down(const LG::Point<int>& location) override { }

private:
    LG::PixelBitmap m_icon;
};