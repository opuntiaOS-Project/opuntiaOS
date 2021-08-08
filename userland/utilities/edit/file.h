#ifndef _USERLAND_EDIT_FILE_H
#define _USERLAND_EDIT_FILE_H

#ifdef __opuntiaOS__
#include <sys/types.h>
#else
#include <stdint.h>
#endif


struct file_view_area {
    char* buffer;
    int start; /* offset in buffer where the line starts */
    int offset; /* offset in file */
    int data_len;
    int buf_len;
    int line;
    char was_changed;
    int fd;
};
typedef struct file_view_area file_view_area_t;

int file_open(char* path);
void file_exit();
int file_save();
int file_line_len(int offset);
int file_line_len_backwards(int offset);
int file_paste_char(char c, int offset);
int file_scroll_up();
int file_scroll_down();

#endif