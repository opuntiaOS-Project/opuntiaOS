#include "pthread.h"
#include "syscalls.h"

int pthread_create(void* func)
{
    mmap_params_t stack_params;
    stack_params.size = 4096; // one page for now)
    stack_params.flags = MAP_STACK;
    stack_params.prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    uint32_t start = mmap(&stack_params);

    thread_create_params_t params;    
    params.stack_start = start;
    params.stack_size = 4096;
    params.entry_point = (uint32_t)func;
    return system_pthread_create(&params);
}