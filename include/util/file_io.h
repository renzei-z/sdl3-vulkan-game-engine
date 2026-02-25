#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <core/cpp_header_guard.h>

HEADER_BEGIN

#include <stddef.h>
#include <stdint.h>

uint8_t *read_entire_file(const char *path, size_t *size);

HEADER_END

#endif // FILE_IO_H_
