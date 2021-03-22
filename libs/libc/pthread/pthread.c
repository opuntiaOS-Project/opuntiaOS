#include <pthread.h>
#include <sys/mman.h>
#include <syscalls.h>

int pthread_create(void* func)
{
    uint32_t start = (uint32_t)mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_STACK | MAP_PRIVATE, 0, 0);
    thread_create_params_t params;
    params.stack_start = start;
    params.stack_size = 4096;
    params.entry_point = (uint32_t)func;
    return system_pthread_create(&params);
}