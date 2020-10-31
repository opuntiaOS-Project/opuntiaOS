#ifndef __oneOS__LibC__SETJMP_H
#define __oneOS__LibC__SETJMP_H

#include <sys/types.h>

struct __jmp_buf {
    uint32_t regs[6];
};

typedef struct __jmp_buf jmp_buf[1];
typedef struct __jmp_buf sigjmp_buf[1];

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int val);

#endif