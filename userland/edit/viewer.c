#include "viewer.h"
#include <libsystem/syscalls.h>

int cursor_x, cursor_y;

void viewer_clear_screen()
{
    cursor_x = cursor_y = 0;
    write(STDOUT, "\x1b[2J", 4);
    write(STDOUT, "\x1b[H", 3);
}

void _viewer_set_cursor_to_start()
{
    write(STDOUT, "\x1b[H", 3);
}

void _viewer_new_line()
{
    write(STDOUT, "\n", 1);
}

bool _viewer_has_space_to_print(char c)
{
    if (c == '\n') {
        if (cursor_y == SCREEN_Y - 1) {
            return false;
        }
    } else {
        if (cursor_x == SCREEN_X - 1 && cursor_y == SCREEN_Y - 1) {
            return false;
        }
    }
    return true;
}

bool _viewer_display_one(char c)
{
    if (_viewer_has_space_to_print(c)) {
        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
            _viewer_new_line();
        } else {
            write(STDOUT, &c, 1);
            cursor_x++;
            if (cursor_x == SCREEN_X) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        return true;
    }
    return false;
}

void viewer_display(char* buf, int len)
{
    viewer_clear_screen();
    for (int i = 0; i < len; i++) {
        if (!_viewer_display_one(buf[i])) {
            return;
        }
    }

    while (cursor_y < SCREEN_Y - 1) {
        cursor_y++;
        write(STDOUT, "\n~", 2);
    }
}

void viewer_enter_menu_mode()
{
    write(STDOUT, "\x1b[1;25H", 7);
}

void viewer_leave_menu_mode()
{
    /* FIXME: Restore cursor to prev position */
    _viewer_set_cursor_to_start();
}