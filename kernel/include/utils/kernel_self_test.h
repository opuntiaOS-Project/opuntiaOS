#ifndef __oneOS__UTILS__KST_H
#define __oneOS__UTILS__KST_H

#include <types.h>

void kpanic(char *t_err_msg);
void kpanic_at_test(char *t_err_msg, uint16_t test_no);
bool kernel_self_test(bool throw_kernel_panic);

#endif // __oneOS__UTILS__KST_H
