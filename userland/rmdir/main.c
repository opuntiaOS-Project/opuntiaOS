#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: rmdir files...\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (rmdir(argv[i]) < 0) {
            printf("rmdir: failed to delete\n");
            break;
        }
    }

    return 0;
}