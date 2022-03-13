#include <unistd.h>

#define initsystem_path "/System/initsystem"

int main()
{
    execlp(initsystem_path, initsystem_path, NULL);
    return 0;
}