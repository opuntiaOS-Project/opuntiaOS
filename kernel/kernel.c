#include "../drivers/display/display.c"

void main() {
    
    char* mem = 0xb8000;
    mem[0] = 'X';
    mem[1] = 0xf0;
    mem[2] = 'C';
    mem[3] = 0xf0;
    
    clean_screen();
    //for (int i = 0; i < 80 * 25; i++) {
        //print_char((char)('0' + (i/80)%10), WHITE_ON_BLACK, -1, -1);
    //}
}