#ifndef __EDIT__FILE__
#define __EDIT__FILE__

struct file_view_area {
    int fd;
    int line;
    int start; /* offset in buffer where the line starts */
    int offset; /* offset in file */
    int data_len;
};
typedef struct file_view_area file_view_area_t;

int file_open(char* path);
void file_exit();
int file_scroll_up(int lines);
int file_scroll_down(int lines);

#endif