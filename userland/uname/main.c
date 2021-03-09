#include <string.h>
#include <syscalls.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd, i;

    utsname_t uts;
    int rc = uname(&uts);
    if (rc < 0) {
        return 1;
    }

    int flag_s = 0;
    int flag_n = 0;
    int flag_r = 0;
    int flag_m = 0;
    if (argc == 1) {
        flag_s = 1;
    } else {
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] == '-') {
                for (const char* o = &argv[i][1]; *o; ++o) {
                    switch (*o) {
                    case 's':
                        flag_s = 1;
                        break;
                    case 'n':
                        flag_n = 1;
                        break;
                    case 'r':
                        flag_r = 1;
                        break;
                    case 'm':
                        flag_m = 1;
                        break;
                    case 'a':
                        flag_s = flag_n = flag_r = flag_m = 1;
                        break;
                    }
                }
            }
        }
    }

    if (!flag_s && !flag_n && !flag_r && !flag_m) {
        flag_s = 1;
    }

    if (flag_s) {
        write(1, " ", 1);
        write(1, uts.sysname, strlen(uts.sysname));
    }
    if (flag_n) {
        write(1, " ", 1);
        write(1, uts.nodename, strlen(uts.nodename));
    }
    if (flag_r) {
        write(1, " ", 1);
        write(1, uts.release, strlen(uts.release));
    }
    if (flag_m) {
        write(1, " ", 1);
        write(1, uts.machine, strlen(uts.machine));
    }
    return 0;
}