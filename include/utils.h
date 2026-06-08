#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(_DEBUG)

#include <assert.h>

/*
  The following macros are for use in debug builds only. They will be automatically
  removed in release builds.
*/
#define UNIMPLEMENTED()\
    do { fprintf(stderr, "REACHED UNIMPLEMENTED @ %s:%d\n", __FILE__, __LINE__); abort(); } while (0)
#define UNREACHABLE()\
    do { fprintf(stderr, "REACHED UNREACHABLE @ %s:%d\n", __FILE__, __LINE__); abort(); } while (0)
#define ASSERT(cond)\
    assert(cond)
#define DEBUG(...)\
    __VA_ARGS__

#elif defined(NDEBUG)

#define UNIMPLEMENTED()
#define UNREACHABLE()
#define ASSERT(cond)
#define DEBUG(...)

#else

#error "_DEBUG or NDEBUG must be defined."

#endif

/*
  Reports an error to the user and quits the program.

  Important to note that this macro is for reporting USER errors, NOT programmer
  errors, use the ASSERT macro for that.
*/
#define ERROR(...) do { fprintf(stderr, "ERROR: " __VA_ARGS__); abort(); } while (0)

/*
  Memory management macros to make calls to standard memory allocation functions more
  elegant. Aborts on failure.

  Do NOT check the return value for NULL, that is already handled internally.
*/
#define MALLOC(size) _mute_malloc(size, __FILE__, __LINE__)
#define CALLOC(num, size) _mute_calloc(num, size, __FILE__, __LINE__)
#define REALLOC(ptr, size) _mute_realloc(ptr, size, __FILE__, __LINE__)
#define FREE(ptr) free(ptr)

// DO NOT USE THESE FUNCTIONS
void* _mute_malloc(size_t size, char* file, int line);
void* _mute_calloc(size_t num, size_t size, char* file, int line);
void* _mute_realloc(void* ptr, size_t size, char* file, int line);

/*
  Reads a file to an array of uint8_t.

  Parameters:
  - char* path       - relative or absolute path to the file to be read
  - size_t* out_size - size of the array

  Returns an array of uint8_t containing the bytes of the provided file.
*/
uint8_t* read_file_to_bytes(char* path, size_t* out_size);

/*
  Reads a file to a null-terminated array of char. Use only for text files.

  Parameters:
  - char* path - relative or absolute path to the file to be read.

  Returns an array of char containing the characters of the provided text file.
*/
char* read_file_to_string(char* path);

#endif
