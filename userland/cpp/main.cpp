#include <libui/App.h>
#include <libui/View.h>
#include <libui/Button.h>
#include <libui/Window.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    fork();
    new UI::App();
    auto* window_ptr = new UI::Window();
    window_ptr->set_superview(new UI::View(window_ptr->bounds()));
    auto* button = new UI::Button({0, 0, 10, 10});
    window_ptr->superview()->add_subview(button);
    button->set_title("oneOS button");
    window_ptr->superview()->set_needs_display();
    window_ptr->set_title("App");
    return UI::App::the().run();
}