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
#define _jblen (6 * 4)
#elif defined(__x86_64__)
/**
 * x86_64 (8 * 8):
 *   - rip, rbp, rsp, rbx, r12, r13, r14, r15
 */
#define _jblen (8 * 8)
#elif defined(__arm__)
/**
 * ARMv7 (11 * 4 + 8 * 8):
 *   - r4 - r12, sp, lr
 *   - d8-d15
 */
#define _jblen (11 * 4 + 8 * 8)
#elif defined(__aarch64__)
/**
 * ARMv8 (22 * 8):
 *   - x19-x28, x29(fp), x30(lr), x31(sp), x4(alignment)
 *   - d8-d15
 */
#define _jblen (22 * 8)
#endif

struct __jmp_buf {
    char regs[_jblen];
};

typedef struct __jmp_buf jmp_buf[1];
typedef struct __jmp_buf sigjmp_buf[1];

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int val);

__END_DECLS

#endif