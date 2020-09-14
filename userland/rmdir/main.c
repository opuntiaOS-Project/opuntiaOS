#include <libsystem/syscalls.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        write(1, "Usage: rmdir files...\n", 22);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (rmdir(argv[i]) < 0) {
            write(1, "rmdir: failed to delete\n", 24);
            break;
        }
    }

    return 0;
}