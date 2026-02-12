#include <util.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t *load_shader_file(const char *file_path, size_t *out_size) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR] Could not open shader file '%s': %s\n",
            file_path, strerror(errno));
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    if (size % 4 != 0) {
        fprintf(stderr, "[ERROR] Shader file '%s' is not aligned to 4 bytes.\n", file_path);
        fclose(f);
        return NULL;
    }

    uint32_t *buf = (uint32_t*)malloc(size);
    if (!buf) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for shader: %s\n", file_path);
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buf, 1, size, f);
    fclose(f);

    if (bytes_read != size) {
        fprintf(stderr, "[ERROR] Failed to read entire shader file '%s': %s\n", file_path, strerror(errno));
        free(buf);
        return NULL;
    }

    *out_size = size;
    return buf;
}