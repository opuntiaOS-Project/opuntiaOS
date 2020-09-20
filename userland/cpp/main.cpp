#include <string.h>
#include <syscalls.h>

class Log {
public:
    static void print(char* buf)
    {
        int len = 0;
        while (buf[len] != '\0')
            len++;
        write(0, buf, len);
    }

    static void print(char* buf, int len)
    {
        write(0, buf, len);
    }
};

int main(int argc, char** argv)
{
    Log::print("hello");
    Log::print("hello", 4);
    return 0;
}