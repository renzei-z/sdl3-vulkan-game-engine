#include <util/file_io.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *read_entire_file(const char *path, size_t *size) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fprintf(stderr, "[ERROR] Could not open file '%s': %s\n",
	    path, strerror(errno));
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t fsize = ftell(file);
  rewind(file);

  uint8_t *buffer = (uint8_t*)malloc(fsize);
  if (!buffer) {
    fprintf(stderr, "[ERROR] Could not allocate buffer to read file '%s'.\n", path);
    fclose(file);
    return NULL;
  }

  if (fread(buffer, 1, fsize, file) != fsize) {
    fprintf(stderr, "[ERROR] Could not read file '%s': %s.\n", path, strerror(errno));
    fclose(file);
    return NULL;
  }
  
  fclose(file);

  *size = fsize;
  return buffer;
}
