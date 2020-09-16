#ifndef __EDIT__VIEWER__
#define __EDIT__VIEWER__

#define STDIN 0
#define STDOUT 1
#define SCREEN_X 80
#define SCREEN_Y 24

void viewer_clear_screen();
void viewer_display(char* buf, int start, int len);
void viewer_enter_menu_mode();
void viewer_cursor_left();
void viewer_cursor_right();
void viewer_cursor_up();
void viewer_cursor_down();
void viewer_cursor_next();
int viewer_get_cursor_offset_in_file();

#endif