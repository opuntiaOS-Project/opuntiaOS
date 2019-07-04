#include "../drivers/display/display.c"

void main() {
    clean_screen();
    print_char('X', BLACK_ON_WHITE, 0, 0);
    print_char('X', BLACK_ON_WHITE, 1, 1);
    print_char('e', BLACK_ON_WHITE, 2, 2);
    print_char('l', BLACK_ON_WHITE, 3, 3);
    print_char('l', BLACK_ON_WHITE, 4, 4);
}