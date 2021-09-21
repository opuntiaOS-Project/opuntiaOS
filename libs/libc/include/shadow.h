#ifndef _LIBC_SHADOW_H
#define _LIBC_SHADOW_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

struct spwd {
    char* sp_namp;
    char* sp_pwdp;
    int sp_lstchg;
    int sp_min;
    int sp_max;
    int sp_warn;
    int sp_inact;
    int sp_expire;
    uint32_t sp_flag;
};

typedef struct spwd spwd_t;

void setspent();
void endspent();
spwd_t* getspent();
spwd_t* getspnam(const char* name);

__END_DECLS

#endif // _LIBC_SHADOW_H