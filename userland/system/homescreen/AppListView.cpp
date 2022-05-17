#include "AppListView.h"
#include <libg/ImageLoaders/PNGLoader.h>
#include <libui/App.h>
#include <libui/Context.h>
#include <libui/Label.h>
#include <libui/Screen.h>
#include <libui/Window.h>

AppListView::AppListView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
    // TODO: Drag-like animation is needed here.
    auto& label = add_subview<UI::Label>(bounds());
    label.set_text("Swipe up to open AppList");
    label.set_alignment(UI::Text::Alignment::Center);
}

void AppListView::display(const LG::Rect& rect)
{
}

void AppListView::on_click()
{
    int this_window_id = window()->id();

    if (m_target_window_id == INVALID) {
        return;
    }

    auto& app = UI::App::the();
    AskBringToFrontMessage msg(app.connection().key(), this_window_id, m_target_window_id);
    app.connection().send_async_message(msg);
}