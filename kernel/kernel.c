#include "../drivers/display/display.c"

void main() {
    cleanScreen();
    printChar('X', BLACK_ON_WHITE, 1, 1);
}