#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdint.h>

#define free_and_set_null(x) { \
    free(x);                   \
    x = NULL;                  \
} 

uint32_t *load_shader_file(const char *file_path, size_t *out_size);

#endif // UTIL_H_