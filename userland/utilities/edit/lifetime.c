#include "lifetime.h"
#ifndef __oneOS__
#include <stdlib.h>
#endif

struct termios orig_term;

void enable_raw_mode()
{
    struct termios raw;
    tcgetattr(STDIN, &raw);
    orig_term = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN, TCSAFLUSH, &raw);
}

void restore_termios()
{
    tcsetattr(STDIN, TCSAFLUSH, &orig_term);
}

void exit_app()
{
    restore_termios();
    exit(0);
}

void crash_app()
{
    restore_termios();
    exit(1);
}
