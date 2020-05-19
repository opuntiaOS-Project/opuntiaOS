#include <libsystem/syscalls.h>

int main() {
    int tmp = the_best_lib_func();
    while (1) {
        print(tmp);
    }
}