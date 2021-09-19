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

int setuid(uid_t uid)
{
    return DO_SYSCALL_1(SYS_SETUID, uid);
}

int setgid(gid_t gid)
{
    return DO_SYSCALL_1(SYS_SETGID, gid);
}

int setreuid(uid_t ruid, uid_t euid)
{
    return DO_SYSCALL_2(SYS_SETREUID, ruid, euid);
}

int setregid(gid_t rgid, gid_t egid)
{
    return DO_SYSCALL_2(SYS_SETREGID, rgid, egid);
}

static char loginbuf[128];
char* getlogin()
{
    passwd_t* passwd = getpwuid(getuid());
    strncpy(loginbuf, passwd->pw_name, 128);
    endpwent();
    return loginbuf;
}