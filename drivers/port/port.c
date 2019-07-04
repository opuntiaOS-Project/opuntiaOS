unsigned char port_byte_in(unsigned short port) {
    unsigned char result_data;
    __asm__("in %%dx, %%al" : "=a"(result_data) : "d"(port));
    return result_data
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("in %%al, %%dx" : "a"(data) : "d"(port));
}

unsigned short port_byte_in(unsigned short port) {
    unsigned short result_data;
    __asm__("in %%dx, %%ax" : "=a"(result_data) : "d"(port));
    return result_data
}

void port_byte_out(unsigned short port, unsigned short data) {
    __asm__("in %%ax, %%dx" : "a"(data) : "d"(port));
}