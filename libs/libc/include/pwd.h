#ifndef _LIBC_PWD_H
#define _LIBC_PWD_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

struct passwd {
    char* pw_name;
    char* pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char* pw_gecos;
    char* pw_dir;
    char* pw_shell;
};
typedef struct passwd passwd_t;

void setpwent();
void endpwent();
passwd_t* getpwent();
passwd_t* getpwuid(uid_t uid);
passwd_t* getpwnam(const char* name);

__END_DECLS

#endif // _LIBC_PWD_H