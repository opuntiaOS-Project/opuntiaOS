#include <iostream>
#include <libfoundation/ProcessInfo.h>
#include <libui/App.h>
#include <libui/AppDelegate.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <memory>

extern "C" bool __init_app_delegate(UI::AppDelegate** res);

// Libs are compiled with -ffreestanding and LLVM mangles
// main() with this flag. Have to mark it as extern "C".
extern "C" int main(int argc, char** argv)
{
    auto process_info = LFoundation::ProcessInfo(argc, argv);
    auto& app = std::oneos::construct<UI::App>();
    UI::AppDelegate* app_delegate = nullptr;
    int res = __init_app_delegate(&app_delegate);
    if (res < 0) {
        return res;
    }

    app.set_delegate(app_delegate);
    app.set_state(UI::AppState::Active);
    int status = app.run();
    app_delegate->application_will_terminate();
    return status;
}