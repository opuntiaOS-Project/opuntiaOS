#include "lifetime.h"

termios_t orig_term;

void enable_raw_mode()
{
    termios_t raw;
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
