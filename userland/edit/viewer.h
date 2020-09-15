#ifndef __EDIT__VIEWER__
#define __EDIT__VIEWER__

#define STDIN 0
#define STDOUT 1
#define SCREEN_X 80
#define SCREEN_Y 24

void viewer_clear_screen();
void viewer_display(char* buf, int len);
void viewer_enter_menu_mode();

#endif