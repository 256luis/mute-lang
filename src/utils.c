#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"

void* _mute_malloc(size_t size, char* file, int line)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "malloc failed @ %s:%d\n", file, line);
        exit(1);
    }

    return ptr;
}

void* _mute_calloc(size_t num, size_t size, char* file, int line)
{
    void* ptr = calloc(num, size);
    if (ptr == NULL)
    {
        fprintf(stderr, "calloc failed @ %s:%d\n", file, line);
        exit(1);
    }

    return ptr;
}

void* _mute_realloc(void* ptr, size_t size, char* file, int line)
{
    void* tmp = realloc(ptr, size);
    if (tmp == NULL)
    {
        fprintf(stderr, "realloc failed @ %s:%d\n", file, line);
        exit(1);
    }

    return tmp;
}

uint8_t* read_file_to_bytes(char* path, size_t* out_size)
{
    int err;

    FILE* file = fopen(path, "rb");
    if (file == NULL) return NULL;

    // get file size
    size_t size;
    err = fseek(file, 0, SEEK_END);
    if (err != 0) return NULL;
    size = ftell(file);

    // put file pointer back to start of file
    fseek(file, 0, SEEK_SET);
    if (err != 0) return NULL;

    // allocate and write bytes
    uint8_t* bytes = MALLOC(size);
    fread(bytes, 1, size, file);
    fclose(file);

    *out_size = size;

    return bytes;
}

char* read_file_to_string(char* path)
{
    int err;

    FILE* file = fopen(path, "r");
    if (file == NULL) return NULL;

    // get file size
    size_t size;
    err = fseek(file, 0, SEEK_END);
    if (err != 0) return NULL;
    size = ftell(file);

    // put file pointer back to start of file
    fseek(file, 0, SEEK_SET);
    if (err != 0) return NULL;

    // allocate and write bytes
    // +1 for null terminator
    char* bytes = CALLOC(size + 1, sizeof(char));

    for (int i = 0, c = fgetc(file); c != EOF; i++, c = fgetc(file))
    {
        bytes[i] = c;
    }

    fclose(file);
    return bytes;
}
