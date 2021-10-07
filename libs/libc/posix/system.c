#include <string.h>
#include <sys/utsname.h>
#include <sysdep.h>
#include <unistd.h>

int uname(utsname_t* buf)
{
    int res = DO_SYSCALL_1(SYS_UNAME, buf);
    RETURN_WITH_ERRNO(res, 0, -1);
}

#define PATH_CONSTANT "/bin:/usr/bin"
#define PATH_CONSTANT_LEN sizeof(PATH_CONSTANT)
size_t confstr(int name, char* buf, size_t len)
{
    switch (name) {
    case _CS_PATH:
        if (!buf || !len) {
            return PATH_CONSTANT_LEN;
        } else {
            // Return path only if enough space.
            if (len < PATH_CONSTANT_LEN) {
                return 0;
            } else {
                memcpy(buf, PATH_CONSTANT, PATH_CONSTANT_LEN);
                return PATH_CONSTANT_LEN;
            }
        }

    default:
        break;
    }

    return 0;
}