#include <assert.h>
#include <pwd.h>
#include <string.h>
#include <sysdep.h>
#include <unistd.h>

uid_t getuid()
{
    return (uid_t)DO_SYSCALL_0(SYS_GETUID);
}

uid_t geteuid()
{
    return 0;
}

static char loginbuf[128];
char* getlogin()
{
    passwd_t* passwd = getpwuid(getuid());
    strncpy(loginbuf, passwd->pw_name, 128);
    endpwent();
    return loginbuf;
}