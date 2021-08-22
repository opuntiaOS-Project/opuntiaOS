#include "file.h"
#include "lifetime.h"
#include "menu.h"
#include "mode.h"
#include "viewer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STDIN 0
#define STDOUT 1

uint32_t mode;
mode_disc_t mode_disc;

void viewing_accept_key(char key)
{
    if (key == ':') {
        menu_enter_mode();
        return;
    }
    if (key == 'w') {
        viewer_cursor_up();
        return;
    }
    if (key == 's') {
        viewer_cursor_down();
        return;
    }
    if (key == 'a') {
        viewer_cursor_left();
        return;
    }
    if (key == 'd') {
        viewer_cursor_right();
        return;
    }

    file_paste_char(key, viewer_get_cursor_offset_in_file());
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