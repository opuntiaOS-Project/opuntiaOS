#include "file.h"
#include "lifetime.h"
#include "menu.h"
#include "mode.h"
#include "viewer.h"
#include <libsystem/string.h>
#include <libsystem/syscalls.h>

#define STDIN 0
#define STDOUT 1

uint32_t mode;
mode_disc_t mode_disc;

void viewing_accept_key(char key)
{
    if (key == ':') {
        menu_enter_mode();
        return true;
    }
    if (key == 'w') {
        file_scroll_up(1);
        return true;
    }
    if (key == 's') {
        file_scroll_down(1);
        return true;
    }
}

char get_key()
{
    char c;
    if (read(STDIN, &c, 1) != 1) {
        crash_app();
    }
    return c;
}

void process_input()
{
    for (;;) {
        char key = get_key();
        mode_disc.accept_key(key);
    }
}

void display_file(char* path)
{
    int fd = file_open(path);
}

void init_mode()
{
    mode = VIEWING;
    mode_disc.accept_key = viewing_accept_key;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        return 0;
    }
    enable_raw_mode();
    init_mode();
    display_file(argv[1]);
    process_input();
    exit_app();
}