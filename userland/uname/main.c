#include <string.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    int fd, i;

    utsname_t uts;
    int rc = uname(&uts);
    if (rc < 0) {
        return 1;
    }

    int flag_s = false;
    int flag_n = false;
    int flag_r = false;
    int flag_m = false;
    if (argc == 1) {
        flag_s = true;
    } else {
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] == '-') {
                for (const char* o = &argv[i][1]; *o; ++o) {
                    switch (*o) {
                    case 's':
                        flag_s = true;
                        break;
                    case 'n':
                        flag_n = true;
                        break;
                    case 'r':
                        flag_r = true;
                        break;
                    case 'm':
                        flag_m = true;
                        break;
                    case 'a':
                        flag_s = flag_n = flag_r = flag_m = true;
                        break;
                    }
                }
            }
        }
    }

    if (!flag_s && !flag_n && !flag_r && !flag_m) {
        flag_s = true;
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