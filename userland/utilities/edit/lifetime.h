#ifndef _USERLAND_EDIT_LIFETIME_H
#define _USERLAND_EDIT_LIFETIME_H

#include "viewer.h"
#include <termios.h>

extern struct termios orig_term;

void enable_raw_mode();
void restore_termios();
void exit_app();
void crash_app();

#endif