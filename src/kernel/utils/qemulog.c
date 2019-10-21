#include <qemulog.h>

void log(const char *message) {
    int i = 0;
    while (message[i] != 0) {
        port_byte_out(COM1, message[i]);
        i++;
    }
}