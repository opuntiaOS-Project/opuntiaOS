#include <syscalls.h>
#include <termios.h>

int tcgetpgrp(int fd)
{
    return ioctl(fd, TIOCGPGRP, 0);
}

int tcsetpgrp(int fd, pid_t pgid)
{
    return ioctl(fd, TIOCSPGRP, pgid);
}

int tcgetattr(int fd, termios_t* termios_p)
{
    return ioctl(0, TCGETS, (int)termios_p);
}

int tcsetattr(int fd, int optional_actions, const termios_t* termios_p)
{
    if (optional_actions == TCSANOW) {
        return ioctl(0, TCSETS, (int)termios_p);
    }
    if (optional_actions == TCSADRAIN) {
        return ioctl(0, TCSETSW, (int)termios_p);
    }
    if (optional_actions == TCSAFLUSH) {
        return ioctl(0, TCSETSF, (int)termios_p);
    }
    return -1;
}