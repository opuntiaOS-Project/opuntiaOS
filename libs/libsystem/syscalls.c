void print(int value) {
    asm volatile ("push %%ebx; movl $0, %%eax; movl %1, %%ebx; int $0x80; pop %%ebx" :: "r"(value), "r"(value) : "memory");
}

int the_best_lib_func() {
    return 4;
}