#include "AppListWindow.h"
#include "AppListView.h"

void AppListWindow::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    Window::receive_event(std::move(event));
}