#include "file.h"
#include "viewer.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

file_view_area_t view_area;

static int _file_get_size_of_first_line()
{
    int res = 0;
    for (int i = view_area.start; i < view_area.data_len; i++) {
        res++;
        if (view_area.buffer[i] == '\n') {
            return res;
        }
    }
    return view_area.data_len - view_area.start;
}

static void _file_find_start(uint32_t old_offset)
{
    old_offset -= 2;
    while (old_offset > view_area.offset) {
        if (view_area.buffer[old_offset - view_area.offset] == '\n') {
            view_area.start = old_offset - view_area.offset + 1;
            return;
        }
        old_offset--;
    }
    view_area.start = 0;
}

static inline int _file_remaining_size()
{
    return view_area.data_len - view_area.start;
}

static inline int _file_is_last_line_in_buf()
{
    return 1;
}

int _file_buf_flush()
{
    /* FIXME: for now we think that we have all file in our buffer */
    lseek(view_area.fd, view_area.offset, SEEK_SET);
    view_area.data_len = write(view_area.fd, view_area.buffer, view_area.data_len);
    return 0;
}

int _file_buf_reload(int offset_in_file)
{
    /* TODO: Get real file size */
    view_area.buf_len = 4000;
    view_area.buffer = (char*)malloc(view_area.buf_len);

    view_area.start = 0;
    view_area.offset = offset_in_file;
    view_area.was_changed = 0;
    lseek(view_area.fd, view_area.offset, SEEK_SET);
    view_area.data_len = read(view_area.fd, view_area.buffer, view_area.buf_len);
    if (view_area.data_len < 0) {
        return -1;
    }
    return 0;
}

int file_line_len(int offset)
{
    int res = 0;
    for (int i = offset; i < view_area.data_len; i++) {
        res++;
        if (view_area.buffer[i] == '\n') {
            return res;
        }
    }
    return view_area.data_len - view_area.start;
}

int file_line_len_backwards(int offset)
{

    int res = 1;
    for (int i = offset - 1; i >= view_area.offset; i--) {
        if (view_area.buffer[i] == '\n') {
            return res;
        }
        res++;
    }
    return offset;
}

int file_open(char* path)
{
    int n;
    if ((view_area.fd = open(path, O_RDWR)) < 0) {
        return -1;
    }

    _file_buf_reload(0);
    if (view_area.data_len < 0) {
        return -1;
    }

    viewer_display(view_area.buffer, view_area.start, _file_remaining_size());

    return view_area.fd;
}

void file_exit()
{
    close(view_area.fd);
}

/**
 * file_paste_char pastes and flush updates to disk.
 */
int file_paste_char(char c, int offset)
{
    view_area.was_changed = 1;
    view_area.data_len++;

    memmove(view_area.buffer + offset + 1, view_area.buffer + offset, view_area.data_len - offset - 1);
    view_area.buffer[offset] = c;
    viewer_cursor_next();
    viewer_display(view_area.buffer, view_area.start, _file_remaining_size());
}

/**
 * file_paste_char pastes and flush updates to disk.
 */
int file_save()
{
    if (view_area.was_changed) {
        _file_buf_flush();
    }
}

/**
 * Scroll functions update file buffer, to contain line scrolled to.
 */
int file_scroll_up()
{
    if (view_area.line == 0 || view_area.start == 0) {
        return -1;
    }
    view_area.line--;
    int old_offset_in_file = view_area.offset + view_area.start;
    int offset_in_file = view_area.offset + view_area.start - SCREEN_X;
    if (offset_in_file < 0) {
        offset_in_file = 0;
    }

    if (offset_in_file >= view_area.offset) {
        view_area.start = offset_in_file - view_area.offset;
        _file_find_start(old_offset_in_file);
        viewer_display(view_area.buffer, view_area.start, _file_remaining_size());
    } else {
        _file_buf_reload(offset_in_file);
        _file_find_start(old_offset_in_file);
        viewer_display(view_area.buffer, view_area.start, _file_remaining_size());
    }
}

int file_scroll_down()
{
    int line_size = _file_get_size_of_first_line();
    view_area.start += line_size;
    if (!_file_is_last_line_in_buf()) {
        _file_buf_reload(view_area.offset + view_area.start);
    }
    viewer_display(view_area.buffer, view_area.start, _file_remaining_size());
    view_area.line++;
}