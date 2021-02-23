#include <stdio.h>
#include <syscalls.h>

size_t _find_free_slot();

size_t _find_free_slot()
{
    for (size_t i = 0; i < MAX_OPENED_FILES; i++) {
        if (opened_files[i]._fileno == -1) {
            return i;
        }
    }

    return -1;
}

FILE* fopen(const char* filename, const char* mode)
{
    size_t free_slot = _find_free_slot();
    if (free_slot == -1) {
        return 0;
    }
    opened_files[free_slot]._fileno = open(filename, 1);
    return &opened_files[free_slot];
}

int fseek(FILE* stream, uint32_t offset, int whence)
{
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return read(stream->_fileno, ptr, size);
}