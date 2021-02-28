#include <syscalls.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        write(1, "Usage: rm files...\n", 21);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (unlink(argv[i]) < 0) {
            write(1, "rm: failed to delete\n", 21);
            break;
        }
    }

    return 0;
}