#include "menu.h"
#include "file.h"
#include "lifetime.h"
#ifdef __oneOS__
#include <syscalls.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

char cmd[SCREEN_X];
int pos = 0;

void menu_run_cmd(char* str, int len)
{
    if (str[0] == 'q' && len == 1) {
        exit_app();
    }
    if (str[0] == 'w' && len == 1) {
        file_save();
    }
    if (str[0] == 'w' && str[1] == 'q' && len == 2) {
        file_save();
        exit_app();
    }
}

void menu_enter_mode()
{
    mode = MENU;
    mode_disc.enter_mode = menu_enter_mode;
    mode_disc.leave_mode = menu_leave_mode;
    mode_disc.accept_key = menu_accept_key;
    viewer_enter_menu_mode();
    write(STDOUT, ": ", 2);

    pos = 0;
}

void menu_leave_mode()
{
}

void menu_accept_key(char key)
{
    if (key == '\n') {
        menu_run_cmd(cmd, pos);
        pos = 0;
        return;
    }
    cmd[pos++] = key;
    write(STDOUT, &key, 1);
}