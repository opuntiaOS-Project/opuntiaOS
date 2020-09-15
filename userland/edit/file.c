#include "file.h"
#include "viewer.h"
#include <libsystem/syscalls.h>

#define CACHE_AREA (SCREEN_X * 3)

char buf[SCREEN_X * SCREEN_Y + 2 * CACHE_AREA];
file_view_area_t view_area;

static int _file_get_size_of_first_line()
{
    int res = 0;
    for (int i = view_area.start; i < view_area.data_len; i++) {
        res++;
        if (buf[i] == '\n') {
            return res;
        }
    }
    return view_area.data_len - view_area.start;
}

static void _file_find_start(uint32_t old_offset)
{
    old_offset -= 2;
    while (old_offset > view_area.offset) {
        if (buf[old_offset - view_area.offset] == '\n') {
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
    return _file_remaining_size() >= (SCREEN_X * SCREEN_Y);
}

/**
 * Reloads buffer and updates view_area.
 */
int _file_buf_reload(int offset_in_file)
{
    uint32_t cache_area = CACHE_AREA;
    if (cache_area > offset_in_file) {
        cache_area = offset_in_file;
    }

    view_area.start = cache_area;
    view_area.offset = offset_in_file - cache_area;
    lseek(view_area.fd, view_area.offset, SEEK_SET);
    view_area.data_len = read(view_area.fd, buf, sizeof(buf));
    if (view_area.data_len < 0) {
        return -1;
    }
    return 0;
}

int file_open(char* path)
{
    int n;
    if ((view_area.fd = open(path, O_RDWR)) < 0) {
        return;
    }

    view_area.start = 0;
    view_area.line = 0;
    view_area.data_len = n;

    view_area.data_len = read(view_area.fd, buf, sizeof(buf));
    view_area.offset = 0;
    if (view_area.data_len < 0) {
        return -1;
    }

    viewer_display(buf + view_area.start, _file_remaining_size());

    return view_area.fd;
}

void file_exit()
{
    close(view_area.fd);
}

/**
 * Scroll functions update file buffer, to contain line scrolled to.
 */
int file_scroll_up(int lines)
{
    if (view_area.line == 0 || view_area.start == 0) {
        return;
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
        viewer_display(buf + view_area.start, _file_remaining_size());
    } else {
        _file_buf_reload(offset_in_file);
        _file_find_start(old_offset_in_file);
        viewer_display(buf + view_area.start, _file_remaining_size());
    }
}

int file_scroll_down(int lines)
{
    int line_size = _file_get_size_of_first_line();
    view_area.start += line_size;
    if (!_file_is_last_line_in_buf()) {
        _file_buf_reload(view_area.offset + view_area.start);
    }
    viewer_display(buf + view_area.start, _file_remaining_size());
    view_area.line++;
}