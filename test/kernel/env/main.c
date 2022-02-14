#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (getenv("OSTEST")) {
        char* res = getenv("GO");
        if (!res) {
            TestErr("Empty GO env var.");
        }
        if (strncmp(res, "go", 2)) {
            TestErr("Different value in GO env var.");
        }
        return 0;
    }

    if (argc > 1) {
        TestErr("Loop while executing test.");
    }

    setenv("OSTEST", "1", 1);
    setenv("GO", "go", 1);
    execlp(argv[0], argv[0], "loop", NULL);
    return 1;
}