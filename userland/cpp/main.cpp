#include <libui/App.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    fork();
    new UI::App();
    auto* window_ptr = new UI::Window();
    window_ptr->set_superview(new UI::View(window_ptr->bounds()));
    window_ptr->superview()->add_subview(new UI::View({10, 10, 50, 50}));
    window_ptr->superview()->set_needs_display();
    window_ptr->set_title("App");
    return UI::App::the().run();
}