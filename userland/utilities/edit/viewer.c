#include "viewer.h"
#include "file.h"
#include <unistd.h>
#include <stdlib.h>
#endif

int out_x, out_y;
int cursor_x, cursor_y;
char* displaying_buffer;
int working_with_offset;

static int _viewer_get_offset(int cx, int cy)
{
    return cy * SCREEN_X + cx;
}

void viewer_clear_screen()
{
    out_x = out_y = 0;
    write(STDOUT, "\x1b[2J", 4);
    write(STDOUT, "\x1b[H", 3);
}

void _viewer_set_cursor(int col, int row)
{
    char buf[8];
    buf[0] = '\x1b';
    buf[1] = '[';
    int id = 2;
    col++, row++;
    if (row < 10) {
        buf[id++] = row + '0';
    } else {
        buf[id++] = (row / 10) + '0';
        buf[id++] = (row % 10) + '0';
    }
    buf[id++] = ';';
    if (col < 10) {
        buf[id++] = col + '0';
    } else {
        buf[id++] = (col / 10) + '0';
        buf[id++] = (col % 10) + '0';
    }
    buf[id++] = 'H';
    write(STDOUT, buf, id);
}

void _viewer_set_cursor_to_start()
{
    write(STDOUT, "\x1b[H", 3);
}

void _viewer_new_line()
{
    write(STDOUT, "\n", 1);
}

char _viewer_has_space_to_print(char c)
{
    if (c == '\n') {
        if (out_y == SCREEN_Y - 1) {
            return 0;
        }
    } else {
        if (out_x == SCREEN_X - 1 && out_y == SCREEN_Y - 1) {
            return 0;
        }
    }
    return 1;
}

char _viewer_display_one(char c)
{
    if (_viewer_has_space_to_print(c)) {
        if (c == '\n') {
            out_x = 0;
            out_y++;
            _viewer_new_line();
        } else {
            write(STDOUT, &c, 1);
            out_x++;
            if (out_x == SCREEN_X) {
                out_x = 0;
                out_y++;
            }
        }
        return 1;
    }
    return 0;
}

static void _log_dec(uint32_t dec)
{
    uint32_t pk = 1000000000;
    char was_not_zero = 0;
    while (pk > 0) {
        uint32_t pp = dec / pk;
        if (was_not_zero || pp > 0) {
            char ccp = pp + '0';
            write(1, &ccp, 1);
            was_not_zero = 1;
        }
        dec -= pp * pk;
        pk /= 10;
    }
    if (!was_not_zero) {
        write(1, "0", 1);
    }
}

void viewer_display(char* buf, int start, int len)
{
    displaying_buffer = buf;
    int last_cx = 0;
    viewer_clear_screen();
    for (int i = start; i < start + len; i++) {
        if (out_x <= cursor_x && out_y == cursor_y) {
            working_with_offset = i;
            last_cx = out_x;
        }
        if (!_viewer_display_one(buf[i])) {
            goto out;
        }
        
    }

    while (out_y < SCREEN_Y - 1) {
        out_y++;
        write(STDOUT, "\n~", 2);
    }
out:
    cursor_x = last_cx;
    _viewer_set_cursor(cursor_x, cursor_y);
}

void viewer_cursor_left()
{
    if (cursor_x) {
        cursor_x--;
        working_with_offset--;
        _viewer_set_cursor(cursor_x, cursor_y);
    }
}

void viewer_cursor_right()
{
    if (displaying_buffer) {
        if (displaying_buffer[working_with_offset] != '\n') {
            cursor_x++;
            working_with_offset++;
            _viewer_set_cursor(cursor_x, cursor_y);
        }
    }
}

void viewer_cursor_up()
{
    if (!cursor_y) {
        file_scroll_up();
    } else {
        working_with_offset -= file_line_len_backwards(working_with_offset);
        cursor_y--;
        int new_line_len = file_line_len_backwards(working_with_offset) - 1;
        if (cursor_x >= new_line_len) {
            cursor_x = (new_line_len) % SCREEN_X;
        }
        working_with_offset -= (new_line_len);
        working_with_offset += cursor_x;
        _viewer_set_cursor(cursor_x, cursor_y);
    }
}

void viewer_cursor_down()
{
    if (cursor_y == SCREEN_Y - 1) {
        file_scroll_down();
    } else {
        working_with_offset += file_line_len(working_with_offset);
        cursor_y++;
        if (cursor_x >= file_line_len(working_with_offset)) {
            cursor_x = file_line_len(working_with_offset) - 1;
        }
        working_with_offset += cursor_x;
        _viewer_set_cursor(cursor_x, cursor_y);
    }
}

void viewer_cursor_next()
{
    cursor_x++;
    if (cursor_x == SCREEN_X) {
        cursor_y++;
        cursor_x = 0;
    }   
}

int viewer_get_cursor_offset_in_file()
{
    return working_with_offset;
}


void viewer_enter_menu_mode()
{
    write(STDOUT, "\x1b[25;1H", 7);
}

void viewer_leave_menu_mode()
{
    /* FIXME: Restore cursor to prev position */
    _viewer_set_cursor_to_start();
}