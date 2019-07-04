//#include "../drivers/display/display.c"

void main() {
    char* video_memory = 0xb8000;
    *video_memory = 'X';
}