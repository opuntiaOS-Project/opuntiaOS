#ifndef _LIBC_SETJMP_H
#define _LIBC_SETJMP_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifdef __i386__
/**
 * x86_32 (6 * 4):
 *   - ebx
 *   - esp
 *   - ebp
 *   - esi
 *   - edi
 *   - return address
 */
#define _jblen (6)
#elif __arm__
/**
 * ARMv7 (11 * 4):
 *   - r4 - r12, sp, lr
 */
#define _jblen (11)
#endif

struct __jmp_buf {
    uint32_t regs[_jblen];
};

typedef struct __jmp_buf jmp_buf[1];
typedef struct __jmp_buf sigjmp_buf[1];

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int val);

__END_DECLS

#endif