#ifndef _KERNEL_LIBKERN_KERNEL_SELF_TEST_H
#define _KERNEL_LIBKERN_KERNEL_SELF_TEST_H

#include <libkern/types.h>

void kpanic_at_test(char* t_err_msg, uint16_t test_no);
bool kernel_self_test(bool throw_kernel_panic);

#endif // _KERNEL_LIBKERN_KERNEL_SELF_TEST_H
