#include <iostream>
#include <libui/App.h>
#include <libui/AppDelegate.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <memory>

extern "C" bool __init_app_delegate();

// Libs are compiled with -ffreestanding and LLVM mangles
// main() with this flag. Have to mark it as extern "C".
extern "C" int main(int argc, char** argv)
{
    auto& app = std::oneos::construct<UI::App>();
    __init_app_delegate();
    return app.run();
}