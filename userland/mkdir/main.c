#include <syscalls.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        write(1, "Usage: mkdir files...\n", 22);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i]) < 0) {
            write(1, "mkdir: failed to create\n", 26);
            return 1;
        }
    }

    return 0;
}