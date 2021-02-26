#ifndef _KERNEL_PLATFORM_X86_TASKING_SWITCHVM_H
#define _KERNEL_PLATFORM_X86_TASKING_SWITCHVM_H

#include <tasking/tasking.h>
#include <libkern/types.h>

void switchuvm(thread_t* thread);

#endif