#ifndef VERTEX_H_
#define VERTEX_H_

#include <core/cpp_header_guard.h>

HEADER_BEGIN

#include <math/math_types.h>

typedef struct vertex_t {
  vec3 pos;
  vec3 color;
  vec2 uv;
} vertex;

HEADER_END

#endif // VERTEX_H_
