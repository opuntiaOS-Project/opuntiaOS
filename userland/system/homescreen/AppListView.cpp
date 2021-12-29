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
    LG::PNG::PNGLoader loader;
    m_icon = loader.load_from_file("/res/system/app_list_32.png");
}

void AppListView::display(const LG::Rect& rect)
{
    const int padding = 4;
    const int offset_x = (bounds().width() - m_icon.width()) / 2;
    ;
    const int offset_y = 0;
    LG::Context ctx = UI::graphics_current_context();
    ctx.add_clip(rect);

    ctx.draw({ offset_x, offset_y }, m_icon);
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