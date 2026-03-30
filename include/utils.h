#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(_DEBUG)

#include <assert.h>

#define UNIMPLEMENTED()\
    { fprintf(stderr, "REACHED UNIMPLEMENTED @ %s:%d\n", __FILE__, __LINE__); exit(1); } (void)0
#define UNREACHABLE()\
    { fprintf(stderr, "REACHED UNREACHABLE @ %s:%d\n", __FILE__, __LINE__); exit(1); } (void)0
#define ASSERT(cond)\
    assert(cond)

#elif defined(NDEBUG)

#define UNIMPLEMENTED()
#define UNREACHABLE()
#define ASSERT(cond)

#else

#error "_DEBUG or NDEBUG must be defined."

#endif

#define ERROR(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(1); } (void)0
#define MALLOC(size) _mute_malloc(size, __FILE__, __LINE__)
#define CALLOC(num, size) _mute_calloc(num, size, __FILE__, __LINE__)
#define REALLOC(ptr, size) _mute_realloc(ptr, size, __FILE__, __LINE__)
#define FREE(ptr) free(ptr)

void* _mute_malloc(size_t size, char* file, int line);
void* _mute_calloc(size_t num, size_t size, char* file, int line);
void* _mute_realloc(void* ptr, size_t size, char* file, int line);
uint8_t* read_file_to_bytes(char* path, size_t* out_size);
char* read_file_to_string(char* path);

#endif
