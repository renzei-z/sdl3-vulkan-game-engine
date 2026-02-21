#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <stddef.h>
#include <stdint.h>

uint8_t *read_entire_file(const char *path, size_t *size);

#endif // FILE_IO_H_
